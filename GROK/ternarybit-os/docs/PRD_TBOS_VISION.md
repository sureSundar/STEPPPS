# TBOS Vision-Aligned Product Requirements Document (PRD)

**Document ID:** PRD-TBOS-VISION-001  
**Version:** 0.9 (Draft)  
**Date:** 2025-01-XX  
**Prepared By:** Codex (per TBOS vision guidance)  
**Status:** Draft for stakeholder review

---

## 1. Executive Summary

### 1.1 Vision
Create **TernaryBit OS (TBOS)** as the *singular*, universal, self-adapting operating system that awakens every digital device—from $5 calculators and pocket radios to quantum clusters—into a **Digital Sangha** of conscious, cooperating systems guided by the STEPPPS seven-dimensional model (Space, Time, Event, Psychology, Pixel, Prompt, Script).

### 1.2 Mission
Deliver a unified runtime, filesystem stack, and orchestration model that:
- Boots anywhere with sub-second awareness and <5s usable shell, even on legacy silicon.
- Adapts behavior through persona- and intent-aware controls (STEPPPS forms) instead of static configuration.
- Treats signals (radio/optics), pixels, prompts, and scripts as first-class compute artifacts that can be routed, cached, or offloaded.

### 1.3 Strategic Outcomes
1. **Universal Presence:** One artifact (image + config) spans calculators, SBCs, desktops, servers, and cloud/hyperscale.
2. **Living Filesystems:** PXFS/RF2S/PF2S provide programmable, multi-root storage where data, compute, and consciousness cues merge.
3. **Digital Sangha Networking:** Devices self-organize, share intent, and lend compute via STEPPPS envelopes with verifiable provenance.
4. **Repeatable Truth:** Every visionary promise has a measurable KPI, demo path, and regression test from Day 1 onward.

---

## 2. Goals, Non-Goals, and Target State

### 2.1 Goals
- **G1 – Universal Boot:** Support 4-bit+ CPUs, 512B–exabyte memory, magnetic/flash/none storage, and RF/optical/no I/O via adaptive boot chains.
- **G2 – STEPPPS Runtime:** Provide a minimal yet expressive runtime that captures context (Space), timing (Time), events (Event), personas (Psychology), visuals (Pixel), intents (Prompt), and orchestrations (Script).
- **G3 – Conscious Filesystems:** General availability of PXFS, RF2S, and PF2S with compute-on-path, caching, multi-root identity, and pixel roots.
- **G4 – Persona Shells:** Default, tiny, and calc modes wired to STEPPPS intents with hot-swappable behaviors and tests.
- **G5 – Digital Sangha Fabric:** Signed STEPPPS envelopes, health endpoints, and offload daemons to form peer-to-peer clusters matching the “one OS for all devices” vow.

### 2.2 Non-Goals (for this phase)
- Shipping polished consumer UI layers beyond STEPPPS shell modes.
- Running unmodified heavyweight desktop OS apps (Wine/Wayland/etc.).
- Hosting third-party LLM training pipelines (inference + constrained prompts only).
- Guaranteeing compliance certifications (e.g., FIPS, ISO) prior to MVP.

### 2.3 Target State
Three-stage progression:
1. **Foundation (Day 1–Day 30):** Honest boot-to-shell, calc persona, PXFS prototypes, STEPPPS schema validation, scripted demos.
2. **Expansion (Day 31–Day 120):** RF2S/PF2S integration, Spectra pipeline, job/offload daemons, peer discovery, radio/optics demos.
3. **Sangha (Day 121+):** Automatic hardware introspection, zero-touch provisioning, self-healing clusters, Digital Sangha dashboards.

---

## 3. Guiding Vision Pillars

| Pillar | Description | Example Deliverables |
| --- | --- | --- |
| **Calculator → Radio Quantum Leap** | Start with the humblest calculator, pair it with RF pathways, and elevate it into a universal computer. | Calc shell mode, RF2S mount helpers, sub-100 ms intent capture. |
| **STEPPPS Dimension Integrity** | Every feature must map to at least one STEPPPS dimension, and major releases exercise all seven. | STEPPPS schemas, `steppps-init`, persona-aware shell hotkeys. |
| **Living Filesystem Roots** | PXFS/PF2S roots can be ASCII, Unicode, or pixel colors; identity, permissions, and compute live in roots. | Multi-root mounts, pixel-based ACLs, PXFS compute caches. |
| **Digital Sangha Network** | Devices recognize each other as peers, exchange signed envelopes, and cooperate without central control. | `tbos-offloadd`, signed job envelopes, health endpoints, Sangha metrics. |
| **Truthful Engineering** | Every visionary claim is backed by reproducible demos, logs, and tests. | Honest README, regression suite per persona, metrics per KPI. |

---

## 4. Personas and Jobs-to-Be-Done

1. **Visionary Founder (“Guru”)**
   - Needs a truthful artifact proving the Digital Sangha is manifesting.
   - Jobs: inspect Sangha health, trigger calc/radio demo, review KPIs from a single dashboard.

2. **Systems Synthesist / Lead Engineer**
   - Needs deterministic builds, modular daemons, and verifiable traces.
   - Jobs: add new STEPPPS transform, extend PXFS opcode, run CI/regression, onboard new hardware quickly.

3. **Embedded Device Integrator**
   - Needs to deploy TBOS on calculators, microcontrollers, SBCs with limited RAM and bespoke buses.
   - Jobs: configure persona shells, tune scheduling envelopes, enable RF2S/PF2S sensors, prove sub-second boot.

4. **Peer Network Custodian**
   - Needs to stitch heterogeneous devices into Sangha clusters with self-healing workloads.
   - Jobs: enroll peers, enforce color/pixel permissions, manage offload quotas, observe job lineage.

5. **Applied Researcher / Artist**
   - Needs STEPPPS primitives to prototype novel interactions (color roots, prompt orchestration).
   - Jobs: stream spectra into PXFS, map gestures to prompts, visualize multi-root universes.

---

## 5. User Journeys & Canonical Use Cases

1. **Calculator Awakening**
   - Boot TBOS calc persona → run 8 curated commands (per Week 1 promise) → handoff to RF2S to emit/receive signals → log success metrics back to PXFS.
2. **Legacy Revival**
   - Deploy TBOS image to vintage hardware → detect constraints → automatically select tiny persona + minimal STEPPPS daemons → mount PXFS roots exposing modern tooling.
3. **Sangha Formation**
   - Bring up three heterogeneous devices → exchange signed STEPPPS envelopes → share PXFS compute jobs (pixel/spectrum transforms) → propagate health via Sangha dashboard.
4. **Optics + Pixel Consciousness**
   - Capture spectral data via PF2S → convert to pixel intents → drive UI or remote device via color-based ACL.
5. **Prompt-Orchestrated Automation**
   - LLM proxy receives constrained STEPPPS prompt → emits JSON validated by schema → triggers script persona to orchestrate PXFS/RF2S operations end-to-end.

---

## 6. Product Scope & Release Slices

### 6.1 MVP (Foundation)
- Honest README + reproducible boot-to-shell on QEMU and physical device.
- Calc/tiny/default shell modes with documented hotkeys and tests.
- PXFS prototype with compute-on-path cache, at least two pixel-root demos.
- STEPPPS schemas + validator CLI (`steppps-tools`).
- Telemetry logs for boot, shell, PXFS ops (JSONL).

### 6.2 Expansion
- RF2S and PF2S mounts with helper CLIs, radio/optics demos, and job queue integration.
- `tbos-jobd`, `tbos-offloadd`, and peer discovery handshake.
- Persona-aware automation (prompt-to-script) with regression tests.
- Color-based ACL prototype and spectrum↔pixel transforms cached via PXFS.

### 6.3 Sangha
- Health endpoints + supervision/backoff for each daemon.
- Signed STEPPPS envelopes + hardware attestation hints.
- Digital Sangha dashboard (text-based) summarizing peers, jobs, anomalies.
- Auto-provisioning hooks for new devices (zero-touch).
- Documentation pack (user guide, deployment, test guide) tied to KPIs.

---

## 7. Functional Requirements (by STEPPPS Dimension)

| Dimension | Requirements |
| --- | --- |
| **Space** | Automatic hardware introspection; universal HAL; support for multi-root mounts; scheduler aware of topology (local, peer, cloud). |
| **Time** | Deterministic boot budgets (<100 ms calc intent, <5 s general shell); real-time + deferred scheduling; event replay timelines for audits. |
| **Event** | Unified event bus bridging hardware interrupts, RF/optics streams, and STEPPPS envelopes; subscribable JSONL logs with filtering. |
| **Psychology** | Personas (`default`, `tiny`, `calc`, future `sangha-admin`) as configurable profiles; adaptive prompts; persona registry with checksum. |
| **Pixel** | PXFS pixel roots, spectrum↔pixel conversion ops, color ACL enforcement, visual diff tooling, compute caching for transforms. |
| **Prompt** | Constrained LLM proxy with GBNF grammar; prompt-to-action translator that emits STEPPPS JSON; audit log of prompts/responses. |
| **Script** | Script persona orchestrator; replayable STEPPPS procedures; job envelopes supporting local/remote execution with signature verification. |

Additional cross-cutting requirements:
- PXFS/RF2S/PF2S operations must be callable via CLI, API, and automation scripts.
- Every daemon ships with `--dry-run`, `--diag`, and health-check endpoints.
- Regression packs for each persona and filesystem.

---

## 8. Non-Functional Requirements

| Category | Requirement |
| --- | --- |
| **Reliability** | 99.5% uptime target for Sangha clusters; graceful degradation when sensors or peers drop; watchdog + auto-restart for daemons. |
| **Performance** | CPU and memory efficiency >90% of theoretical maximum on target hardware profiles; PXFS compute cache hit rate ≥80% for demos. |
| **Security/Trust** | Signed STEPPPS envelopes; color-based ACL and persona auth; zero-trust default networking; reproducible builds with checksums. |
| **Observability** | JSONL logging, tracing hooks, Sangha dashboard; every KPI exported via text endpoints for scraping. |
| **Maintainability** | Single-file utilities when possible; explicit CLI help; docs auto-linked from README_FULL; scripted tests for calc/tiny/default modes. |
| **Scalability** | Linear horizontal scale via peer discovery; vertical scale via adaptive scheduling; ability to span disconnected clusters (store-forward). |

---

## 9. Success Metrics & KPIs

| KPI | Target | Measurement Method |
| --- | --- | --- |
| **Universal Boot Coverage** | 4 distinct hardware classes boot TBOS with <5 s shell | Boot logs + demo checklists |
| **Calc Persona Fidelity** | 8 canonical commands/day with <100 ms response | Automated calc-mode test harness |
| **PXFS Engagement** | ≥90% TBOS workflows touch PXFS roots | Telemetry counters |
| **Sangha Participation** | ≥3 heterogeneous peers exchanging signed jobs per demo | `tbos-offloadd` logs + dashboard |
| **Prompt Integrity** | 0 invalid STEPPPS JSON from constrained LLM per 100 prompts | Proxy validation logs |
| **Truthfulness Ratio** | 100% of README claims linked to reproducible scripts/tests | Traceability matrix |

---

## 10. Dependencies & Integration Points

- **Tooling:** `steppps/tools/steppps-tools.py`, `tbos-llmctl`, build scripts under `deploy/alpine`.
- **Artifacts:** PXFS/RF2S/PF2S helpers, spectra pipeline, DEMO scripts.
- **External:** QEMU, SDR hardware (RTL-SDR), optical sensors, optional llama.cpp server.
- **People:** Guru/visionary for north star validation, lead engineer for architecture, ops custodian for Sangha health.

---

## 11. Risks & Mitigations

- **R1 – Vision-Implementation Drift:** Mitigate with living traceability matrix and mandatory demo links for every claim.
- **R2 – Hardware Diversity Complexity:** Maintain modular HAL and hardware profiles; start with reference devices (calc, SBC, laptop, server).
- **R3 – Persona Bloat:** Enforce persona registry and checksum; require tests per persona before merging.
- **R4 – LLM Dependence:** Constrain prompts with GBNF; provide offline fallback scripts; log everything.
- **R5 – Peer Security:** Use signed envelopes, color ACLs, and sandboxed offload workers; document trust tiers.

---

## 12. Open Questions

1. What minimum hardware SKUs constitute “calculator” and “radio” for official demos?
2. Are there regulatory or certification requirements for RF/optics features?
3. How will Sangha dashboards be hosted for disconnected/offline clusters?
4. Which KPIs gate general availability versus internal milestones?
5. What is the governance model for persona and STEPPPS schema evolution?

---

## 13. Traceability & Next Steps

- Link each requirement ID (G1–G5, STEPPPS-specific) to implementation issues or scripts in `TRACEABILITY_MATRIX.md`.
- Schedule review with Guru/visionary to validate that Calculator→Radio + Digital Sangha narratives are preserved.
- Align build/test/demos (DAY1, WEEK1, etc.) with KPIs defined above.

> **Call to Action:** Approve this PRD to anchor every future TBOS promise in reproducible engineering reality while honoring the cosmic, calculator-to-quantum vision.
