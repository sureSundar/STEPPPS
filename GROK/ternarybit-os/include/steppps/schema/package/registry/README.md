# TBOS Package Registry

Not yet wired up. `tools/steppps_validate.py` only knows about `dev-action`
and `earth` entities today (`DEV_ACTION_SCHEMA_PATH`, `EARTH_*` constants) -
it has no `PACKAGE_SCHEMA_PATH`, no `admit-package` subcommand, and nothing
in this directory has been formally admitted or hash-chained.

`../examples/pxfs-lossless.package.json` was checked by hand (required
fields present, every `space.paths`/`package.provides[].source` file
verified to exist on disk - see docs/TBOS_LINUX_DISTRO_ROADMAP.md) but that
is not the same as passing through the real validator and landing in a
`chain.jsonl` here the way `include/steppps/schema/dev/registry/` entries do.

Next real step: extend `tools/steppps_validate.py` with package-schema
support (`admit-package`, hash-chain into `chain.jsonl` in this directory),
following the exact pattern already established for `dev` and `earth`, then
admit `pxfs-lossless.package.json` for real.
