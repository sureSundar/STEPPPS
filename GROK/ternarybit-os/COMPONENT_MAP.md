# TBOS + STEPPPS Component Relationship Map

## 🗺️ Visual Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    EXISTING STEPPPS FRAMEWORK                   │
│                    (50,000+ lines, 100+ files)                  │
└─────────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│  SPACE        │    │  TIME         │    │  EVENT        │
│  Manager      │    │  Manager      │    │  Manager      │
│               │    │               │    │               │
│ • Hardware    │    │ • Scheduling  │    │ • I/O Events  │
│ • Resources   │    │ • Deadlines   │    │ • Interrupts  │
│ • Personas    │    │ • Timers      │    │ • Processing  │
└───────┬───────┘    └───────┬───────┘    └───────┬───────┘
        │                    │                     │
        │                    │                     │
        ▼                    ▼                     ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│  PSYCHOLOGY   │    │  PIXEL        │    │  PROMPT       │
│  Manager      │    │  Manager      │    │  Manager      │
│               │    │               │    │               │
│ • AI Learning │    │ • Rendering   │    │ • UI/UX       │
│ • Adaptation  │    │ • Graphics    │    │ • NLP         │
│ • Prediction  │    │ • Display     │    │ • Commands    │
└───────┬───────┘    └───────┬───────┘    └───────┬───────┘
        │                    │                     │
        └────────────────────┼─────────────────────┘
                             │
                             ▼
                    ┌───────────────┐
                    │  SCRIPT       │
                    │  Manager      │
                    │               │
                    │ • Automation  │
                    │ • Plugins     │
                    │ • Workflows   │
                    └───────────────┘
```

---

## 🔄 Component Overlap Analysis

### 1. Command Processing

```
EXISTING (STEPPPS):              MY NEW CODE:              CONSOLIDATION:
┌──────────────────┐            ┌──────────────────┐      ┌──────────────────┐
│ EVENT Dimension  │            │ Command Router   │      │ EVENT Dimension  │
│                  │            │                  │      │                  │
│ • Event queue    │  CONFLICT  │ • Handler table  │  =>  │ • Commands as    │
│ • I/O processing │     ⚠️      │ • AI routing     │      │   events         │
│ • Interrupt mgmt │            │ • Quantum route  │      │ • Uses PSYCH for │
│                  │            │                  │      │   AI routing     │
└──────────────────┘            └──────────────────┘      └──────────────────┘
```

### 2. User Interface

```
EXISTING (STEPPPS):              MY NEW CODE:              CONSOLIDATION:
┌──────────────────┐            ┌──────────────────┐      ┌──────────────────┐
│ PROMPT Dimension │            │ Shell Commands   │      │ PROMPT Dimension │
│                  │            │                  │      │                  │
│ • NLP interface  │  CONFLICT  │ • CLI handlers   │  =>  │ • Enhanced with  │
│ • Voice input    │     ⚠️      │ • Text parsing   │      │   CLI commands   │
│ • Gestures       │            │ • Help system    │      │ • Keeps handlers │
│                  │            │                  │      │                  │
└──────────────────┘            └──────────────────┘      └──────────────────┘
```

### 3. Display Management

```
EXISTING (STEPPPS):              MY NEW CODE:              CONSOLIDATION:
┌──────────────────┐            ┌──────────────────┐      ┌──────────────────┐
│ PIXEL Dimension  │            │ GUI Framework    │      │ PIXEL Dimension  │
│                  │            │                  │      │                  │
│ • Render context │  OVERLAP   │ • Windows/Widget │  =>  │ • Wraps widgets  │
│ • Graphics       │     ⚠️      │ • Themes         │      │ • Theme mapping  │
│ • 2D/3D support  │            │ • Desktop        │      │ • Desktop render │
│                  │            │                  │      │                  │
└──────────────────┘            └──────────────────┘      └──────────────────┘
```

### 4. AI Integration

```
EXISTING (STEPPPS):              MY NEW CODE:              CONSOLIDATION:
┌──────────────────┐            ┌──────────────────┐      ┌──────────────────┐
│ PSYCHOLOGY Dim   │            │ AI Routing       │      │ PSYCHOLOGY Dim   │
│                  │            │                  │      │                  │
│ • Learning       │ DUPLICATE  │ • AI command     │  =>  │ • All AI through │
│ • Adaptation     │     ❌      │   routing        │      │   PSYCHOLOGY     │
│ • Prediction     │            │ • Confidence     │      │ • Remove dups    │
│                  │            │                  │      │                  │
└──────────────────┘            └──────────────────┘      └──────────────────┘
```

### 5. Hardware Abstraction

```
EXISTING (STEPPPS):              MY NEW CODE:              CONSOLIDATION:
┌──────────────────┐            ┌──────────────────┐      ┌──────────────────┐
│ SPACE Dimension  │            │ Persona System   │      │ SPACE Dimension  │
│                  │            │                  │      │                  │
│ • HW detection   │    USE     │ • 9 personas     │  =>  │ • Manage 9       │
│ • Resource mgmt  │     ✅      │ • Switching      │      │   personas       │
│ • Personas       │            │ • State save     │      │ • Use SPACE API  │
│                  │            │                  │      │                  │
└──────────────────┘            └──────────────────┘      └──────────────────┘
```

---

## 📊 Dependency Graph

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                    │
│                                                         │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────┐ │
│  │   Shell      │    │     GUI      │    │   Apps   │ │
│  │   Commands   │    │   Widgets    │    │          │ │
│  └──────┬───────┘    └──────┬───────┘    └────┬─────┘ │
└─────────┼──────────────────┼──────────────────┼───────┘
          │                  │                  │
          ▼                  ▼                  ▼
┌─────────────────────────────────────────────────────────┐
│              STEPPPS Framework (Core Layer)             │
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐ │
│  │  PROMPT  │  │  PIXEL   │  │  EVENT   │  │ PSYCH  │ │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └───┬────┘ │
│       │             │              │             │      │
│       └─────────────┼──────────────┼─────────────┘      │
│                     ▼              ▼                     │
│            ┌────────────────────────────┐               │
│            │  STEPPPS Coordinator       │               │
│            │  (7-Dimension Sync)        │               │
│            └────────────┬───────────────┘               │
└─────────────────────────┼───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                   Hardware Layer                        │
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐ │
│  │   CPU    │  │  Memory  │  │  Display │  │  I/O   │ │
│  └──────────┘  └──────────┘  └──────────┘  └────────┘ │
└─────────────────────────────────────────────────────────┘
```

---

## 🔍 Code Flow Analysis

### Current Flow (Problematic)

```
User Command
    │
    ▼
┌──────────────────┐
│ My Shell Manager │ ──────┐
└──────────────────┘       │
    │                      │
    ▼                      │ BYPASSES STEPPPS!
┌──────────────────┐       │         ❌
│ Command Router   │◄──────┘
└──────────────────┘
    │
    ▼
┌──────────────────┐
│ Direct Execution │
└──────────────────┘
```

### Correct Flow (After Consolidation)

```
User Command
    │
    ▼
┌─────────────────────┐
│ PROMPT Dimension    │ (Parse input, NLP)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ PSYCHOLOGY Dim      │ (AI analysis if needed)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ EVENT Dimension     │ (Route as event)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ SPACE/TIME/SCRIPT   │ (Execute in appropriate dimension)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ PIXEL/PROMPT        │ (Display result)
└─────────────────────┘
```

---

## 📋 Integration Matrix

| Feature | Current Location | Should Use | Priority |
|---------|------------------|------------|----------|
| Command parsing | tbos_shell_manager | PROMPT | 🔴 HIGH |
| Command routing | command_router | EVENT | 🔴 HIGH |
| AI decision | command_router | PSYCHOLOGY | 🔴 HIGH |
| Persona switch | shell_manager | SPACE | 🟡 MEDIUM |
| GUI rendering | gui_framework | PIXEL | 🟡 MEDIUM |
| Task scheduling | (missing) | TIME | 🟢 LOW |
| Automation | (missing) | SCRIPT | 🟢 LOW |

---

## 🏗️ Refactoring Workflow

### Step-by-Step Integration

```
Phase 1: Shell Commands → PROMPT Dimension
├── 1.1: Move command parsing to PROMPT
├── 1.2: Keep command handlers (help, persona, etc.)
├── 1.3: Route handler calls through PROMPT API
└── 1.4: Test all 6 commands

Phase 2: Command Routing → EVENT Dimension
├── 2.1: Convert commands to STEPPPS events
├── 2.2: Use EVENT queue for processing
├── 2.3: Remove duplicate routing code
└── 2.4: Test event processing

Phase 3: AI Features → PSYCHOLOGY Dimension
├── 3.1: Move AI routing to PSYCHOLOGY
├── 3.2: Use PSYCHOLOGY prediction API
├── 3.3: Remove duplicate AI code
└── 3.4: Test AI routing

Phase 4: GUI → PIXEL Dimension
├── 4.1: Map GUI widgets to STEPPPS objects
├── 4.2: Use PIXEL render context
├── 4.3: Integrate themes with PIXEL
└── 4.4: Test rendering

Phase 5: Personas → SPACE Dimension
├── 5.1: Use SPACE persona detection
├── 5.2: Migrate persona switching to SPACE
├── 5.3: Remove duplicate persona code
└── 5.4: Test all 9 personas
```

---

## 📁 File Migration Plan

### Files to Keep (Useful Utilities)

| File | Why Keep | Integration |
|------|----------|-------------|
| tbos_shell_implementation.c | Command handlers | Call via PROMPT |
| tbos_gui_implementation.c | GUI helpers | Wrap with PIXEL |
| tbos_simple_demo.c | Standalone demo | Works independently |

### Files to Refactor

| Old File | New File | Changes |
|----------|----------|---------|
| tbos_shell_manager.c | tbos_steppps_shell.c | Use STEPPPS dimensions |
| tbos_command_router.c | (integrate into EVENT) | Use EVENT dimension |
| tbos_gui_framework.c | tbos_steppps_gui.c | Use PIXEL dimension |
| tbos_integrated_demo.c | tbos_steppps_demo.c | Full STEPPPS integration |

### Files to Remove (Duplicates)

- None yet - First refactor, then identify true duplicates

---

## 🎯 Success Metrics

### Before Consolidation
- ❌ Shell bypasses STEPPPS
- ❌ GUI doesn't use PIXEL
- ❌ AI routing duplicated
- ❌ Personas not using SPACE
- ❌ No dimension coordination

### After Consolidation
- ✅ Shell uses PROMPT + EVENT
- ✅ GUI uses PIXEL dimension
- ✅ AI through PSYCHOLOGY only
- ✅ Personas via SPACE dimension
- ✅ Full 7-dimension coordination

---

## 📚 Reference Documentation

### Must Read Before Refactoring
1. `STEPPPS_OS_ARCHITECTURE.md` - Overall design
2. `STEPPPS_BOOTLOADER_DESIGN.md` - Boot integration
3. `src/core/steppps_framework.h` - Core API
4. `CONSOLIDATION_PLAN.md` - This plan

### Code References
- PROMPT API: `src/core/steppps_dimensions/prompt_manager.h`
- EVENT API: `src/core/steppps_dimensions/event_manager.h`
- PIXEL API: `src/core/steppps_dimensions/pixel_manager.h`
- SPACE API: `src/core/steppps_dimensions/space_manager.h`
- PSYCHOLOGY API: `src/core/steppps_dimensions/psychology_manager.h`

---

**Created**: 2025-10-29
**Status**: PLANNING PHASE
**Next**: Begin Shell refactoring using STEPPPS
