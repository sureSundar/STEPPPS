# Earth Entity Registry

This directory is the first creation-time gate for Earth entities.

An entity should enter this registry only through:

```text
python3 tools/steppps_validate.py admit-earth <entity.json>
```

The gate validates the entity against its `kind` schema, checks that its
`steppps://...` ID has not already been admitted, and writes a stable registry
filename derived from the entity kind and ID.

Before admission, the gate also executes the local smart contract:

```text
include/steppps/schema/earth/contracts/admission-owner.v1.contract.json
```

The contract is deterministic JSON policy. It defines allowed Earth kinds,
external reference prefixes, required known registry references, and privileged
authority rules.

Each successful admission also appends a block to:

```text
chain.jsonl
```

The chain stores the admitted entity hash, previous block hash, and current
block hash. Each new block also records the contract that governed admission.
It is not a currency. It is a small tamper-evident memory for the Earth
registry.

Verify it with:

```text
python3 tools/steppps_validate.py verify-earth-chain
```

Examples remain under `include/steppps/schema/earth/examples/`. Registry files
represent admitted entities.

Signature: CX
