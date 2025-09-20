# TBOS × STEPPPS: A Universal, Autonomously Orchestrated Operating Substrate for Phygital Systems

Authors: Suresh Sundararajan (concept, product vision), Cascade (engineering), community contributors
Date: 2025-09-20

Abstract
We present TBOS (TernaryBit OS), a reliability-first substrate that operationalizes the STEPPPS model—Space, Time, Event, Psychology, Pixel, Prompt, Script—into a practical, on-device, low-footprint operating environment with graceful offload. TBOS unifies constrained generation (GBNF) for LLM outputs, content-addressed compute over a path-encoded filesystem (PXFS), radio/optical signal file systems (RF2S, PF2S), and an adaptive shell persona model to enable predictable, inspectable autonomy on devices ranging from calculators to servers. We demonstrate an optics pipeline (PF2S) and STEPPPS-aware transforms, job queuing with peer offload, and a Micro LLM that writes valid STEPPPS frames to RF2S/PXFS. Results show that (1) constrained decoding dramatically improves JSON validity and downstream composability; (2) compute-on-read with caching transforms CPU-bound tasks into I/O-limited retrieval; and (3) persona-controlled UX enforces graceful degradation while preserving observability.

1. Introduction
Autonomous systems demand explainability and reliability under resource constraints. STEPPPS offers a holistic data/intent model—unifying spatial, temporal, perceptual, cognitive, and procedural layers. TBOS embodies this model as an OS substrate: small tools, explicit artifacts, predictable failure modes. This paper argues for filesystem-first autonomy: if everything is a path and an artifact, then introspection, caching, and offload become universal.

2. Related Work
- Functional dataflow systems and build tools (Bazel, Nix) emphasize content-addressed results; TBOS adapts the idea to live devices via PXFS.
- LLM constrained decoding (GBNF, JSON mode) enhances structured output reliability; TBOS standardizes GBNF for STEPPPS.
- SDR pipelines treat RF as data; TBOS generalizes to RF2S and PF2S to unify radio/optical I/O as files.
- Edge orchestration often relies on heavyweight agents; TBOS pursues minimal single-file tools for each role.

3. System Design
3.1 STEPPPS Data Model
- Frames: versioned, timestamped JSON objects with optional events/links; links may point to PXFS/RF2S/PF2S artifacts.
- Validated by JSON schema; produced by constrained LLM or rules.

3.2 Filesystem-First Abstractions
- PXFS: a virtual path namespace encoding operations and inputs. Reading a path computes the result on miss; results are cached by op+inputs hash.
- RF2S/PF2S: domain-specific trees rooted by frequency/wavelength. Mount helpers present user-friendly views; backing stores ensure availability.

3.3 Micro LLM
- `tbos-llm-proxy` fronts llama.cpp with a STEPPPS GBNF grammar; writes frames to PXFS/RF2S and validates against schemas. Fallback mode produces minimal valid frames when the model is unavailable.

3.4 Job System and Offload
- `tbos-jobd` polls `/var/lib/tbos/jobs` and `/mnt/px/jobs` for JSON jobs and materializes them via `tbos-compute`. Offload to `tbos-offloadd` peers when persona/policy dictates.

3.5 Personas and Profiles
- `tbos-shell` personas: default (full), tiny (compact), calc (calculator). UX adapts without changing the substrate.
- Profiles can raise/lower quotas or switch kernels (future work) for burst compute.

4. Implementation
- Single-file tools in `deploy/alpine/`: init, shell, sensors, FS FUSE drivers, LLM proxy/CLI, compute/job/offload.
- STEPPPS toolchain in `steppps/tools/steppps-tools.py`: transforms (pixel ops, spectrum features, pixel↔spectrum, wave.fft2) and validation.
- Demos in `deploy/alpine/qa/`: QEMU boot, optics demos.

5. Results
5.1 Constrained LLM → Valid STEPPPS
- Using `steppps_json.gbnf` cut invalid JSON outputs to near-zero in our tests, unblocking automation pipelines (validation, rendering, transforms).

5.2 Compute as Caching
- PXFS ops (e.g., spectrum.features, pixel.from_spectrum) resolve to cached artifacts after first materialization, reducing CPU by orders of magnitude on constrained devices.

5.3 RF2S/PF2S as First-Class I/O
- RF/optical artifacts are discoverable and scriptable paths; this enables human-inspectable autonomy and cross-device remoting via simple file transfer.

6. Discussion
- Inspectability: By letting everything be a file and a path, TBOS reduces the cognitive gap between intent and dataflow. JSONL logs provide time-indexed audit trails.
- Reliability: Fallbacks (LLM rules), validation, and persona gating produce predictable degradation instead of failure.
- Extensibility: New ops and sensors are a file away. The grammar/schema layer evolves with STEPPPS versioning.

7. Ethics and Spirituality
- The project intentionally blends science and spirituality. The humility of "graceful degradation" mirrors dharmic ethics: act responsibly within capacity, escalate transparently, and avoid harm.
- Privacy and Consent: Offload and sensing should be policy-gated and user-consented; signatures for jobs are recommended.

8. Limitations
- The pixel↔spectrum mappings rely on simplified Gaussian primaries; calibrated SPDs improve accuracy.
- Offload security is minimal; production requires signed envelopes and mutual auth.
- The wave.fft2 transform is educational; performance is limited by naive DFT.

9. Future Work
- Health endpoints and supervision backoff; signed job envelopes; peer discovery.
- Richer transforms and calibrated colorimetry; optics modulation with camera/photodiode drivers.
- Multi-STEPPPS orchestration and negotiation across devices.

10. Conclusion
TBOS demonstrates that a filesystem-first, grammar-constrained, persona-aware substrate can make autonomy inspectable and reliable on tiny devices. By grounding AI outputs in valid schemas and persistent artifacts, we bring life to devices while honoring predictability.

Appendix A: Reproduction
- Build: `deploy/alpine/build-x86_64.sh`
- Boot: `deploy/alpine/qa/qemu-x86_64.sh --kernel <bzImage> --initrd ../out/x86_64/initramfs.cpio.gz`
- Demos: `deploy/alpine/qa/tbos-optic-demo.sh`, `tbos-optic-link-demo.sh`
- Compute ops: `tbos-compute pxfs://compute/...`

Appendix B: Key Files
- `deploy/alpine/common/steppps-init`
- `deploy/alpine/common/tbos-shell`
- `deploy/alpine/llm/tbos-llm-proxy.py`, `tbos-llmctl`, `steppps_json.gbnf`
- `deploy/alpine/compute/tbos-compute.py`, `tbos-jobd.py`, `tbos-offloadd.py`
- `deploy/alpine/fs/*_fuse.py`, `deploy/alpine/fs/helpers/*`
- `steppps/tools/steppps-tools.py`
- `deploy/alpine/qa/*.sh`, `deploy/alpine/qa/README_OPTICS.md`
