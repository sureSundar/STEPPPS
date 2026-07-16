# TBOS Development STEPPPS Registry

This directory is the dogfood registry for TBOS development.

Every meaningful TBOS development step should be represented as a small
`dev-action` STEPPPS record, admitted through:

```text
python3 tools/steppps_validate.py admit-dev <dev-action.json>
```

For fast day-to-day work, create and admit a development record in one command:

```text
python3 tools/steppps_validate.py record-dev <slug> --intent <text> --path <path> --command <cmd> --verify <cmd> --prompt <text>
```

Admission validates the JSON schema, executes the development dogfood contract,
writes a stable registry file, and appends a tamper-evident hash-chain block to:

```text
chain.jsonl
```

Verify the chain with:

```text
python3 tools/steppps_validate.py verify-dev-chain
```

Signature: CX
