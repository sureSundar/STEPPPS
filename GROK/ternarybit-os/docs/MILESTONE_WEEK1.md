# Milestone: Week‑1 Shell (Navigation Baseline)

**Goal:** Prove that TernaryBit OS can boot to the conscious shell and execute the first seven navigation/file commands on both the hosted calc persona and the raw bootable image.

## Scope

| Layer          | In Scope                                                                      | Out of Scope (deferred)                                     |
|----------------|-------------------------------------------------------------------------------|-------------------------------------------------------------|
| Shell          | `help`, `ls`, `cd`, `pwd`, `mkdir`, `rmdir`, `touch`                          | Remaining 393 commands, advanced PXFS/RF2S/PHFS interfaces  |
| Filesystem     | Ramdisk root structure, VFS plumbing required for the above commands          | Full PXFS compression, UCFS cultural overlays beyond basic  |
| Build/Boot     | Hosted personas (`build_tbos_system.sh --persona calc_4k/desktop_512m`) and `build/build_universal.sh` boot image | Super personas, Windows-native harnesses                    |
| QA             | Automated smoke tests for hosted calc persona + scripted QEMU boot smoke test | Long-running Sangha/network tests                           |

Success Criteria:
1. `build_tbos_system.sh --persona calc_4k` and `--persona desktop_512m` build cleanly (no warnings) and `tests/smoke/shell_week1.sh` reports success.
2. `build/build_universal.sh` produces `build/tbos_universal.img` (or calc-profile image) that boots in QEMU via `scripts/test_boot_image.sh`, reaches the shell prompt, runs `help` and `exit` without errors.
3. Weekly commands documented in `docs/COMMAND_PROGRESS.md` marked as complete with a reference to the test run.

## Required Tasks

1. **Shell Navigation Commands**
   - Implement `ls`, `cd`, `pwd`, `mkdir`, `rmdir`, `touch` per acceptance criteria.
   - Add unit tests where practical; ensure hosted smoke test exercises each command.

2. **Filesystem Plumbing**
   - Finish ramdisk path traversal (`find_inode_by_path`) and directory creation/removal logic.
   - Ensure root structure is initialized during boot and accessible to the commands above.

3. **Bootable Build Profile**
   - Extend `build/build_universal.sh` (or wrapper) with a calc-style profile so we can emit a right-sized disk image for testing.
   - Document run commands in `docs/DEPLOYMENT.md`.

4. **Automation**
   - `tests/smoke/shell_week1.sh`: runs the hosted calc persona, feeds the Week‑1 commands, verifies exit codes.
   - `scripts/test_boot_image.sh`: boots the disk image under QEMU (serial console), feeds `help`/`exit`, and returns non-zero on failure.
   - Wire both scripts into CI (`scripts/tbos_persona_ci.sh` + future GitHub/cron job).

## Definition of Done

- All tasks above merged to `main`.
- Persona builds for calc/desktop emit zero compiler/linker warnings (treat warnings as errors going forward).
- Smoke test scripts pass locally and in CI; logs archived.
- Documentation updated (`docs/COMMAND_PROGRESS.md`, `docs/DEPLOYMENT.md`) with instructions/results.

Only after this milestone is marked complete do we proceed to PXFS enhancements, network/Sangha work, or additional shell commands. 
