# TBOS Lifecycle Policy (STEPPPS-Aligned)

The hosted TBOS shell now follows an automatic genesis/demise policy inspired by the STEPPPS dimensions:

| Persona | Max Session | Idle Timeout | Karma Floor | STEPPPS Lens |
|---------|-------------|--------------|-------------|--------------|
| `calc_4k`        | 10 min | 90 s  | 0   | **Space** limits tiny hardware; **Time** enforces short bursts. |
| `mobile_64k`     | 30 min | 180 s | -5  | Mobile nodes sleep quickly to preserve battery. |
| `desktop_512m`   | 60 min | 240 s | -15 | Desktop personas should not hog HAL resources when idle. |
| `super_1g`       | 90 min | 360 s | -25 | Super nodes surrender cycles when karma or time budgets run low. |
| `network_sangha` | 60 min | 180 s | -10 | Sangha persona shuts down when compassion (karma) falters. |
| `fs_universal`   | 60 min | 240 s | -20 | Filesystem persona vacates RAM if unused. |
| default          | 30 min | 180 s | -25 | Catch-all policy. |

## Genesis

At persona launch the lifecycle manager records HAL metrics (Space), schedules time budgets (Time), and emits a STEPPPS system event documenting the new session. Persona builds that don’t declare a specific policy fall back to the default.

## Demise

During the shell loop, the lifecycle manager evaluates:

1. **Idle Time (Time/Event):** if no commands execute within the persona’s idle budget, the shell logs a lifecycle warning and exits.
2. **Session Duration (Time):** sessions exceeding the max runtime terminate gracefully.
3. **Karma Floor (Psychology):** if user karma drops below the persona’s minimum threshold, the session ends to encourage mindful usage.

Each demise path emits a STEPPPS system event. The `persona` shell command can still request a morph, which resets the lifecycle timer for the new persona.

## Rebirth

After demise, the lifecycle manager schedules a persona to respawn:

| Demise Reason Contains | Rebirth Persona        | Rationale |
|------------------------|------------------------|-----------|
| `idle`                | Same persona           | Resume when the user returns. |
| `session`             | `fs_universal`         | Release compute persona, move to lightweight filesystem mode. |
| `karma`               | `network_sangha`       | Encourage compassion reboot. |
| default               | Same persona           | Preserve the prior context. |

When the previous shell exits, the supershell checks for a pending rebirth and automatically launches the queued persona, logging the lifecycle event to STEPPPS. You’ll see a console message such as:

```
[Lifecycle] Rebirth triggered after: Auto shutdown (desktop_512m idle for 240s)
```

You can also call `persona morph <id>` manually; that bypasses the automatic rebirth queue and hands control back to the supershell immediately.

## Karma Ledger

Every karma change now lands in `artifacts/karma_ledger.log` with timestamp, persona, delta, and resulting total. The shell loads the last total at genesis so karma persists across sessions/persona rebirths. Use:

```
karma ledger [entries]
```

to view the last N ledger entries for the current persona (default 10). The ledger records actions such as command costs, rewards, right-speech violations, and lifecycle-triggered karma adjustments.
