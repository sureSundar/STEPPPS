#!/usr/bin/env python3
"""Validate TBOS STEPPPS JSON examples with no third-party dependencies.

This is a small validator for the JSON Schema subset used by the canonical
v1 schemas under include/steppps/schema. It is intentionally conservative:
unknown schema keywords are ignored, while supported keywords fail closed.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import sys
from datetime import UTC, datetime
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
EARTH_SCHEMA_DIR = ROOT / "include" / "steppps" / "schema" / "earth"
EARTH_EXAMPLES_DIR = EARTH_SCHEMA_DIR / "examples"
EARTH_REGISTRY_DIR = EARTH_SCHEMA_DIR / "registry"
EARTH_CHAIN_PATH = EARTH_REGISTRY_DIR / "chain.jsonl"
EARTH_CONTRACTS_DIR = EARTH_SCHEMA_DIR / "contracts"
EARTH_ADMISSION_CONTRACT_PATH = EARTH_CONTRACTS_DIR / "admission-owner.v1.contract.json"
DEV_SCHEMA_DIR = ROOT / "include" / "steppps" / "schema" / "dev"
DEV_EXAMPLES_DIR = DEV_SCHEMA_DIR / "examples"
DEV_ACTION_SCHEMA_PATH = DEV_SCHEMA_DIR / "dev-action.v1.schema.json"
DEV_REGISTRY_DIR = DEV_SCHEMA_DIR / "registry"
DEV_CHAIN_PATH = DEV_REGISTRY_DIR / "chain.jsonl"
DEV_CONTRACT_PATH = DEV_SCHEMA_DIR / "contracts" / "dev-dogfood.v1.contract.json"


class ValidationError(Exception):
    pass


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def canonical_json(value: Any) -> str:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def sha256_text(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def sha256_json(value: Any) -> str:
    return sha256_text(canonical_json(value))


def relative_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def type_name(value: Any) -> str:
    if isinstance(value, bool):
        return "boolean"
    if isinstance(value, dict):
        return "object"
    if isinstance(value, list):
        return "array"
    if isinstance(value, str):
        return "string"
    if isinstance(value, int):
        return "integer"
    if isinstance(value, float):
        return "number"
    if value is None:
        return "null"
    return type(value).__name__


def schema_type_matches(expected: str, value: Any) -> bool:
    if expected == "number":
        return isinstance(value, (int, float)) and not isinstance(value, bool)
    if expected == "integer":
        return isinstance(value, int) and not isinstance(value, bool)
    if expected == "object":
        return isinstance(value, dict)
    if expected == "array":
        return isinstance(value, list)
    if expected == "string":
        return isinstance(value, str)
    if expected == "boolean":
        return isinstance(value, bool)
    if expected == "null":
        return value is None
    return False


def resolve_ref(ref: str, root_schema: dict[str, Any]) -> dict[str, Any]:
    prefix = "#/$defs/"
    if not ref.startswith(prefix):
        raise ValidationError(f"unsupported $ref {ref}")
    name = ref[len(prefix):]
    try:
        target = root_schema["$defs"][name]
    except KeyError as exc:
        raise ValidationError(f"missing $defs entry for {ref}") from exc
    if not isinstance(target, dict):
        raise ValidationError(f"$defs entry for {ref} is not an object")
    return target


def validate_datetime(value: str, path: str) -> None:
    candidate = value[:-1] + "+00:00" if value.endswith("Z") else value
    try:
        datetime.fromisoformat(candidate)
    except ValueError as exc:
        raise ValidationError(f"{path}: expected date-time, got {value!r}") from exc


def validate_value(value: Any, schema: dict[str, Any], root_schema: dict[str, Any], path: str = "$") -> None:
    if "$ref" in schema:
        validate_value(value, resolve_ref(schema["$ref"], root_schema), root_schema, path)
        return

    if "const" in schema and value != schema["const"]:
        raise ValidationError(f"{path}: expected const {schema['const']!r}, got {value!r}")

    if "enum" in schema and value not in schema["enum"]:
        raise ValidationError(f"{path}: expected one of {schema['enum']!r}, got {value!r}")

    expected_type = schema.get("type")
    if expected_type:
        allowed = expected_type if isinstance(expected_type, list) else [expected_type]
        if not any(schema_type_matches(t, value) for t in allowed):
            raise ValidationError(f"{path}: expected {expected_type}, got {type_name(value)}")

    if isinstance(value, str):
        if "minLength" in schema and len(value) < schema["minLength"]:
            raise ValidationError(f"{path}: string shorter than minLength {schema['minLength']}")
        if "pattern" in schema and not re.search(schema["pattern"], value):
            raise ValidationError(f"{path}: string does not match pattern {schema['pattern']!r}")
        if schema.get("format") == "date-time":
            validate_datetime(value, path)

    if isinstance(value, (int, float)) and not isinstance(value, bool):
        if "minimum" in schema and value < schema["minimum"]:
            raise ValidationError(f"{path}: value below minimum {schema['minimum']}")
        if "maximum" in schema and value > schema["maximum"]:
            raise ValidationError(f"{path}: value above maximum {schema['maximum']}")

    if isinstance(value, list):
        if "minItems" in schema and len(value) < schema["minItems"]:
            raise ValidationError(f"{path}: array shorter than minItems {schema['minItems']}")
        item_schema = schema.get("items")
        if isinstance(item_schema, dict):
            for index, item in enumerate(value):
                validate_value(item, item_schema, root_schema, f"{path}[{index}]")

    if isinstance(value, dict):
        required = schema.get("required", [])
        for key in required:
            if key not in value:
                raise ValidationError(f"{path}: missing required property {key!r}")

        properties = schema.get("properties", {})
        if schema.get("additionalProperties") is False:
            allowed_keys = set(properties)
            extra = sorted(set(value) - allowed_keys)
            if extra:
                raise ValidationError(f"{path}: additional properties not allowed: {extra}")

        for key, prop_schema in properties.items():
            if key in value:
                validate_value(value[key], prop_schema, root_schema, f"{path}.{key}")


def schema_for_kind(kind: str) -> Path:
    path = EARTH_SCHEMA_DIR / f"{kind}.v1.schema.json"
    if not path.exists():
        raise ValidationError(f"no Earth schema for kind {kind!r}: {path}")
    return path


def validate_file(path: Path, schema_path: Path | None = None) -> tuple[str, str]:
    instance = load_json(path)
    if not isinstance(instance, dict):
        raise ValidationError(f"{path}: top-level value must be an object")
    if schema_path is None:
        kind = instance.get("kind")
        if not isinstance(kind, str):
            raise ValidationError(f"{path}: missing string kind")
        schema_path = schema_for_kind(kind)
    schema = load_json(schema_path)
    validate_value(instance, schema, schema)
    return str(path), str(schema_path)


def collect_steppps_refs(value: Any) -> list[str]:
    refs: list[str] = []
    if isinstance(value, str) and value.startswith("steppps://"):
        refs.append(value)
    elif isinstance(value, dict):
        for child in value.values():
            refs.extend(collect_steppps_refs(child))
    elif isinstance(value, list):
        for child in value:
            refs.extend(collect_steppps_refs(child))
    return refs


def value_at_path(value: Any, path: str) -> list[Any]:
    current = [value]
    for part in path.split("."):
        next_values: list[Any] = []
        is_array = part.endswith("[]")
        key = part[:-2] if is_array else part
        for item in current:
            if not isinstance(item, dict) or key not in item:
                continue
            child = item[key]
            if is_array:
                if isinstance(child, list):
                    next_values.extend(child)
                else:
                    next_values.append(child)
            else:
                next_values.append(child)
        current = next_values
    return current


def validate_model(directory: Path) -> None:
    files = sorted(directory.glob("*.json"))
    if not files:
        raise ValidationError(f"{directory}: no JSON examples found")

    ids: dict[str, Path] = {}
    docs: list[tuple[Path, dict[str, Any]]] = []
    for path in files:
        validate_file(path)
        doc = load_json(path)
        entity_id = doc.get("id")
        if not isinstance(entity_id, str):
            raise ValidationError(f"{path}: missing string id")
        if entity_id in ids:
            raise ValidationError(f"duplicate id {entity_id!r}: {ids[entity_id]} and {path}")
        ids[entity_id] = path
        docs.append((path, doc))

    external_prefixes = ("steppps://nation/", "steppps://sangha/")
    for path, doc in docs:
        for ref in collect_steppps_refs(doc):
            if ref == doc.get("id"):
                continue
            if ref in ids or ref.startswith(external_prefixes):
                continue
            raise ValidationError(f"{path}: unresolved reference {ref}")


def registry_filename(entity_id: str, kind: str) -> str:
    prefix = f"steppps://{kind}/"
    if not entity_id.startswith(prefix):
        raise ValidationError(f"id {entity_id!r} must start with {prefix!r}")
    name = entity_id[len(prefix):]
    if not re.fullmatch(r"[A-Za-z0-9._-]+", name):
        raise ValidationError(f"id tail {name!r} is not safe as a registry filename")
    return f"{kind}-{name}.json"


def dev_registry_filename(entity_id: str) -> str:
    prefix = "steppps://dev-action/"
    if not entity_id.startswith(prefix):
        raise ValidationError(f"id {entity_id!r} must start with {prefix!r}")
    name = entity_id[len(prefix):]
    if not re.fullmatch(r"[A-Za-z0-9._-]+", name):
        raise ValidationError(f"id tail {name!r} is not safe as a registry filename")
    return f"dev-action-{name}.json"


def safe_slug(value: str) -> str:
    slug = re.sub(r"[^A-Za-z0-9._-]+", "-", value.strip()).strip("-")
    if not slug:
        raise ValidationError("slug must contain at least one safe character")
    return slug


def load_registry_ids(registry_dir: Path) -> dict[str, Path]:
    ids: dict[str, Path] = {}
    if not registry_dir.exists():
        return ids
    for path in sorted(registry_dir.glob("*.json")):
        doc = load_json(path)
        entity_id = doc.get("id") if isinstance(doc, dict) else None
        if not isinstance(entity_id, str):
            raise ValidationError(f"{path}: missing string id")
        if entity_id in ids:
            raise ValidationError(f"duplicate registry id {entity_id!r}: {ids[entity_id]} and {path}")
        ids[entity_id] = path
    return ids


def human_owner_ids(registry_ids: dict[str, Path]) -> set[str]:
    owners: set[str] = set()
    for entity_id, path in registry_ids.items():
        if not entity_id.startswith("steppps://human/"):
            continue
        try:
            doc = load_json(path)
        except (OSError, json.JSONDecodeError):
            continue
        if isinstance(doc, dict) and doc.get("kind") == "human":
            owners.add(entity_id)
    return owners


def enforce_admission_contract(doc: dict[str, Any], registry_ids: dict[str, Path], contract_path: Path) -> str:
    contract = load_json(contract_path)
    if not isinstance(contract, dict):
        raise ValidationError(f"{contract_path}: contract must be an object")
    if contract.get("applies_to") != "earth-admission":
        raise ValidationError(f"{contract_path}: contract does not apply to earth-admission")

    kind = doc.get("kind")
    entity_id = doc.get("id")
    if not isinstance(kind, str) or not isinstance(entity_id, str):
        raise ValidationError("contract: entity missing string kind or id")
    if kind not in contract.get("allowed_kinds", []):
        raise ValidationError(f"contract: kind {kind!r} is not allowed")

    known_ids = set(registry_ids) | {entity_id}
    external_prefixes = tuple(contract.get("external_prefixes", []))
    required_paths = contract.get("required_known_reference_paths", {}).get(kind, [])
    for path in required_paths:
        values = value_at_path(doc, path)
        if not values:
            raise ValidationError(f"contract: required reference path {path!r} is missing")
        for value in values:
            if not isinstance(value, str) or not value.startswith("steppps://"):
                raise ValidationError(f"contract: {path!r} must contain STEPPPS references")
            if value in known_ids or value.startswith(external_prefixes):
                continue
            raise ValidationError(f"contract: unresolved admitted reference {value!r} at {path!r}")

    forbidden_scope = set(contract.get("forbidden_scope_without_human_owner", []))
    if kind == "authority" and forbidden_scope.intersection(doc.get("scope", [])):
        if doc.get("holder") not in human_owner_ids(registry_ids):
            raise ValidationError("contract: privileged authority scope requires an admitted human holder")

    contract_id = contract.get("id")
    if not isinstance(contract_id, str):
        raise ValidationError(f"{contract_path}: missing string contract id")
    return contract_id


def enforce_dev_contract(doc: dict[str, Any], contract_path: Path) -> str:
    contract = load_json(contract_path)
    if not isinstance(contract, dict):
        raise ValidationError(f"{contract_path}: contract must be an object")
    if contract.get("applies_to") != "dev-action":
        raise ValidationError(f"{contract_path}: contract does not apply to dev-action")
    if doc.get("kind") != "dev-action":
        raise ValidationError("contract: dev action must have kind 'dev-action'")

    for dimension in contract.get("required_dimensions", []):
        if dimension not in doc:
            raise ValidationError(f"contract: missing STEPPPS dimension {dimension!r}")

    action = value_at_path(doc, "event.action")
    if not action or action[0] not in contract.get("allowed_actions", []):
        raise ValidationError(f"contract: action {action[0] if action else None!r} is not allowed")

    owner = value_at_path(doc, "meta.owner")
    owner_prefixes = tuple(contract.get("required_owner_prefixes", []))
    if not owner or not isinstance(owner[0], str) or not owner[0].startswith(owner_prefixes):
        raise ValidationError("contract: owner must be a human or agent STEPPPS reference")

    verification = value_at_path(doc, "script.verification[]")
    min_verification = contract.get("minimum_verification_commands", 1)
    if len(verification) < min_verification:
        raise ValidationError("contract: dev action must include verification commands")

    contract_id = contract.get("id")
    if not isinstance(contract_id, str):
        raise ValidationError(f"{contract_path}: missing string contract id")
    return contract_id


def admit_earth_entity(
    path: Path,
    registry_dir: Path,
    replace: bool = False,
    contract_path: Path = EARTH_ADMISSION_CONTRACT_PATH,
) -> Path:
    validate_file(path)
    doc = load_json(path)
    kind = doc.get("kind")
    entity_id = doc.get("id")
    if not isinstance(kind, str) or not isinstance(entity_id, str):
        raise ValidationError(f"{path}: missing string kind or id")

    registry_ids = load_registry_ids(registry_dir)
    if entity_id in registry_ids and not replace:
        raise ValidationError(f"{path}: id already admitted at {registry_ids[entity_id]}")
    contract_id = enforce_admission_contract(doc, registry_ids, contract_path)

    registry_dir.mkdir(parents=True, exist_ok=True)
    target = registry_dir / registry_filename(entity_id, kind)
    if target.exists() and not replace:
        raise ValidationError(f"{path}: target already exists: {target}")

    shutil.copyfile(path, target)
    append_chain_block(path, target, registry_dir, contract_id)
    return target


def load_registry_ids_for_files(registry_dir: Path) -> dict[str, Path]:
    ids: dict[str, Path] = {}
    if not registry_dir.exists():
        return ids
    for path in sorted(registry_dir.glob("*.json")):
        doc = load_json(path)
        entity_id = doc.get("id") if isinstance(doc, dict) else None
        if not isinstance(entity_id, str):
            raise ValidationError(f"{path}: missing string id")
        if entity_id in ids:
            raise ValidationError(f"duplicate registry id {entity_id!r}: {ids[entity_id]} and {path}")
        ids[entity_id] = path
    return ids


def admit_dev_action(
    path: Path,
    registry_dir: Path,
    replace: bool = False,
    contract_path: Path = DEV_CONTRACT_PATH,
) -> Path:
    validate_file(path, DEV_ACTION_SCHEMA_PATH)
    doc = load_json(path)
    if not isinstance(doc, dict):
        raise ValidationError(f"{path}: dev action must be an object")
    entity_id = doc.get("id")
    if not isinstance(entity_id, str):
        raise ValidationError(f"{path}: missing string id")

    registry_ids = load_registry_ids_for_files(registry_dir)
    if entity_id in registry_ids and not replace:
        raise ValidationError(f"{path}: id already admitted at {registry_ids[entity_id]}")
    contract_id = enforce_dev_contract(doc, contract_path)

    registry_dir.mkdir(parents=True, exist_ok=True)
    target = registry_dir / dev_registry_filename(entity_id)
    if target.exists() and not replace:
        raise ValidationError(f"{path}: target already exists: {target}")

    shutil.copyfile(path, target)
    append_dev_chain_block(path, target, registry_dir, contract_id)
    return target


def create_dev_action(
    slug: str,
    action: str,
    intent: str,
    paths: list[str],
    commands: list[str],
    verification: list[str],
    prompt: str,
    owner: str,
    output_dir: Path,
) -> Path:
    slug = safe_slug(slug)
    if not paths:
        raise ValidationError("record-dev requires at least one --path")
    if not commands:
        raise ValidationError("record-dev requires at least one --command")
    if not verification:
        raise ValidationError("record-dev requires at least one --verify")

    doc = {
        "steppps_version": "1.0",
        "kind": "dev-action",
        "id": f"steppps://dev-action/{slug}",
        "space": {
            "realm": "virtual",
            "repo": ROOT.name,
            "paths": paths,
        },
        "time": {
            "created": datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        },
        "event": {
            "action": action,
            "intent": intent,
            "links": [str(DEV_CONTRACT_PATH.relative_to(ROOT))],
        },
        "psychology": {
            "consciousness_level": "aware",
            "karma": {
                "total": 1,
            },
        },
        "pixel": {
            "form": "json",
            "display_logic": "Render this development action as an inspectable JSON record with STEPPPS dimensions visible.",
        },
        "prompt": {
            "source": "guru",
            "text": prompt,
            "next_genai_prompt": "Continue the next TBOS change by creating or updating a STEPPPS dev-action, then verify the dev chain.",
        },
        "script": {
            "acts_on": ["space", "time", "event", "psychology", "pixel", "prompt", "script"],
            "commands": commands,
            "verification": verification,
        },
        "meta": {
            "owner": owner,
            "contract": "steppps://contract/dev-dogfood-v1",
        },
    }

    output_dir.mkdir(parents=True, exist_ok=True)
    path = output_dir / f"{slug}.dev-action.json"
    with path.open("w", encoding="utf-8") as f:
        f.write(json.dumps(doc, indent=2, ensure_ascii=False))
        f.write("\n")
    validate_file(path, DEV_ACTION_SCHEMA_PATH)
    return path


def chain_path_for_registry(registry_dir: Path) -> Path:
    return registry_dir / "chain.jsonl"


def block_hash(block: dict[str, Any]) -> str:
    unsigned = {key: value for key, value in block.items() if key != "hash"}
    return sha256_json(unsigned)


def load_chain(chain_path: Path) -> list[dict[str, Any]]:
    if not chain_path.exists():
        return []
    blocks: list[dict[str, Any]] = []
    with chain_path.open("r", encoding="utf-8") as f:
        for line_number, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            block = json.loads(line)
            if not isinstance(block, dict):
                raise ValidationError(f"{chain_path}:{line_number}: block must be an object")
            blocks.append(block)
    return blocks


def append_chain_block(
    source_path: Path,
    target_path: Path,
    registry_dir: Path,
    contract_id: str | None = None,
) -> dict[str, Any]:
    chain_path = chain_path_for_registry(registry_dir)
    blocks = load_chain(chain_path)
    doc = load_json(target_path)
    if not isinstance(doc, dict):
        raise ValidationError(f"{target_path}: registry entity must be an object")

    block: dict[str, Any] = {
        "version": "earth-registry-chain.v1",
        "index": len(blocks),
        "timestamp": datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "action": "admit-earth",
        "contract": contract_id,
        "entity_id": doc.get("id"),
        "kind": doc.get("kind"),
        "source": relative_path(source_path),
        "target": relative_path(target_path),
        "entity_sha256": sha256_json(doc),
        "previous_hash": blocks[-1]["hash"] if blocks else "0" * 64,
    }
    block["hash"] = block_hash(block)

    chain_path.parent.mkdir(parents=True, exist_ok=True)
    with chain_path.open("a", encoding="utf-8") as f:
        f.write(canonical_json(block) + "\n")
    return block


def append_dev_chain_block(source_path: Path, target_path: Path, registry_dir: Path, contract_id: str) -> dict[str, Any]:
    chain_path = chain_path_for_registry(registry_dir)
    blocks = load_chain(chain_path)
    doc = load_json(target_path)
    if not isinstance(doc, dict):
        raise ValidationError(f"{target_path}: registry dev action must be an object")

    block: dict[str, Any] = {
        "version": "dev-registry-chain.v1",
        "index": len(blocks),
        "timestamp": datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "action": "admit-dev",
        "contract": contract_id,
        "entity_id": doc.get("id"),
        "event_action": value_at_path(doc, "event.action")[0],
        "source": relative_path(source_path),
        "target": relative_path(target_path),
        "entity_sha256": sha256_json(doc),
        "previous_hash": blocks[-1]["hash"] if blocks else "0" * 64,
    }
    block["hash"] = block_hash(block)

    chain_path.parent.mkdir(parents=True, exist_ok=True)
    with chain_path.open("a", encoding="utf-8") as f:
        f.write(canonical_json(block) + "\n")
    return block


def verify_chain(chain_path: Path, expected_version: str = "earth-registry-chain.v1") -> None:
    blocks = load_chain(chain_path)
    previous_hash = "0" * 64
    for expected_index, block in enumerate(blocks):
        if block.get("version") != expected_version:
            raise ValidationError(f"{chain_path}: block {expected_index}: unsupported version")
        if block.get("index") != expected_index:
            raise ValidationError(f"{chain_path}: block {expected_index}: wrong index {block.get('index')!r}")
        if block.get("previous_hash") != previous_hash:
            raise ValidationError(f"{chain_path}: block {expected_index}: previous_hash mismatch")
        if block.get("hash") != block_hash(block):
            raise ValidationError(f"{chain_path}: block {expected_index}: hash mismatch")

        target = ROOT / str(block.get("target"))
        if not target.exists():
            raise ValidationError(f"{chain_path}: block {expected_index}: missing target {target}")
        doc = load_json(target)
        if sha256_json(doc) != block.get("entity_sha256"):
            raise ValidationError(f"{chain_path}: block {expected_index}: entity hash mismatch for {target}")
        previous_hash = str(block["hash"])


def latest_chain_doc(chain_path: Path, expected_version: str) -> tuple[dict[str, Any], dict[str, Any], Path]:
    verify_chain(chain_path, expected_version)
    blocks = load_chain(chain_path)
    if not blocks:
        raise ValidationError(f"{chain_path}: chain is empty")
    block = blocks[-1]
    target = ROOT / str(block.get("target"))
    doc = load_json(target)
    if not isinstance(doc, dict):
        raise ValidationError(f"{target}: target document must be an object")
    return block, doc, target


def print_next_dev(chain_path: Path) -> None:
    block, doc, target = latest_chain_doc(chain_path, "dev-registry-chain.v1")
    next_prompts = value_at_path(doc, "prompt.next_genai_prompt")
    display_logic = value_at_path(doc, "pixel.display_logic")
    acts_on = value_at_path(doc, "script.acts_on[]")
    verification = value_at_path(doc, "script.verification[]")

    print("NEXT DEV STEPPPS")
    print(f"block: {block.get('index')}")
    print(f"entity: {doc.get('id')}")
    print(f"target: {relative_path(target)}")
    print()
    print("PROMPT.next_genai_prompt:")
    print(next_prompts[0] if next_prompts else "")
    print()
    print("PIXEL.display_logic:")
    print(display_logic[0] if display_logic else "")
    print()
    print("SCRIPT.acts_on:")
    for dimension in acts_on:
        print(f"- {dimension}")
    print()
    print("SCRIPT.verification:")
    for command in verification:
        print(f"- {command}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate TBOS STEPPPS JSON files")
    sub = parser.add_subparsers(dest="cmd", required=True)

    file_cmd = sub.add_parser("file", help="validate one JSON file")
    file_cmd.add_argument("path", type=Path)
    file_cmd.add_argument("--schema", type=Path)

    earth_cmd = sub.add_parser("earth-examples", help="validate the tiny Earth example model")
    earth_cmd.add_argument("--dir", type=Path, default=EARTH_EXAMPLES_DIR)

    admit_cmd = sub.add_parser("admit-earth", help="validate and admit one Earth entity into the registry")
    admit_cmd.add_argument("path", type=Path)
    admit_cmd.add_argument("--registry", type=Path, default=EARTH_REGISTRY_DIR)
    admit_cmd.add_argument("--contract", type=Path, default=EARTH_ADMISSION_CONTRACT_PATH)
    admit_cmd.add_argument("--replace", action="store_true")

    chain_cmd = sub.add_parser("verify-earth-chain", help="verify the Earth registry hash chain")
    chain_cmd.add_argument("--chain", type=Path, default=EARTH_CHAIN_PATH)

    dev_cmd = sub.add_parser("admit-dev", help="validate and admit one TBOS development STEPPPS action")
    dev_cmd.add_argument("path", type=Path)
    dev_cmd.add_argument("--registry", type=Path, default=DEV_REGISTRY_DIR)
    dev_cmd.add_argument("--contract", type=Path, default=DEV_CONTRACT_PATH)
    dev_cmd.add_argument("--replace", action="store_true")

    dev_chain_cmd = sub.add_parser("verify-dev-chain", help="verify the TBOS development hash chain")
    dev_chain_cmd.add_argument("--chain", type=Path, default=DEV_CHAIN_PATH)

    next_dev_cmd = sub.add_parser("next-dev", help="print the next GenAI prompt and action hints from the latest dev STEPPPS record")
    next_dev_cmd.add_argument("--chain", type=Path, default=DEV_CHAIN_PATH)

    record_dev_cmd = sub.add_parser("record-dev", help="create and admit a TBOS development STEPPPS action")
    record_dev_cmd.add_argument("slug")
    record_dev_cmd.add_argument("--action", default="implement")
    record_dev_cmd.add_argument("--intent", required=True)
    record_dev_cmd.add_argument("--path", action="append", required=True, dest="paths")
    record_dev_cmd.add_argument("--command", action="append", required=True, dest="commands")
    record_dev_cmd.add_argument("--verify", action="append", required=True, dest="verification")
    record_dev_cmd.add_argument("--prompt", required=True)
    record_dev_cmd.add_argument("--owner", default="steppps://agent/cx")
    record_dev_cmd.add_argument("--out-dir", type=Path, default=DEV_EXAMPLES_DIR)
    record_dev_cmd.add_argument("--registry", type=Path, default=DEV_REGISTRY_DIR)
    record_dev_cmd.add_argument("--replace", action="store_true")

    args = parser.parse_args()
    try:
        if args.cmd == "file":
            path, schema_path = validate_file(args.path, args.schema)
            print(f"OK {path} :: {schema_path}")
        elif args.cmd == "earth-examples":
            validate_model(args.dir)
            print(f"OK earth examples :: {args.dir}")
        elif args.cmd == "admit-earth":
            target = admit_earth_entity(args.path, args.registry, args.replace, args.contract)
            print(f"OK admitted {args.path} -> {target}")
        elif args.cmd == "verify-earth-chain":
            verify_chain(args.chain)
            print(f"OK earth chain :: {args.chain}")
        elif args.cmd == "admit-dev":
            target = admit_dev_action(args.path, args.registry, args.replace, args.contract)
            print(f"OK admitted dev {args.path} -> {target}")
        elif args.cmd == "verify-dev-chain":
            verify_chain(args.chain, "dev-registry-chain.v1")
            print(f"OK dev chain :: {args.chain}")
        elif args.cmd == "next-dev":
            print_next_dev(args.chain)
        elif args.cmd == "record-dev":
            path = create_dev_action(
                args.slug,
                args.action,
                args.intent,
                args.paths,
                args.commands,
                args.verification,
                args.prompt,
                args.owner,
                args.out_dir,
            )
            target = admit_dev_action(path, args.registry, args.replace, DEV_CONTRACT_PATH)
            print(f"OK recorded dev {path} -> {target}")
    except (OSError, json.JSONDecodeError, ValidationError) as exc:
        print(f"ERR {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
