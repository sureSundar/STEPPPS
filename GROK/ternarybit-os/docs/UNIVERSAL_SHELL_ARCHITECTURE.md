# TernaryBit OS - Universal Comprehensive Shell Architecture

**Epic**: Universal Shell
**Story Points**: 150
**Status**: In Progress
**Date**: 2025-11-04

---

## Vision

Create the world's first **truly universal shell** that runs identically on:
- Linux (POSIX/bash)
- Windows (cmd.exe/PowerShell)
- macOS (BSD/zsh)
- Embedded systems (BusyBox)
- TernaryBit OS (all hardware profiles)

**Philosophy**: "One shell to rule them all, one syntax to bind them"

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    User Interface Layer                     │
│  (Interactive REPL, Script Execution, Pipe Support)         │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│              Command Dispatcher & Parser                     │
│  - Syntax detection (POSIX/Windows/macOS)                   │
│  - Command aliasing & translation                           │
│  - Pipe/redirect handling                                    │
│  - Variable expansion                                        │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│           Universal Command Registry (~400 commands)         │
│  ┌────────────┬──────────────┬─────────────────┐            │
│  │ POSIX (200)│ Windows (150)│  macOS/BSD (50) │            │
│  └────────────┴──────────────┴─────────────────┘            │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│              Filesystem Abstraction Layer                    │
│  ┌──────────┬──────────┬──────────┬──────────────┐          │
│  │   PXFS   │   UCFS   │   RF2S   │   Ramdisk    │          │
│  │(Compress)│(Universal)│  (Rock)  │  (Memory)    │          │
│  └──────────┴──────────┴──────────┴──────────────┘          │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                 Hardware Abstraction Layer                   │
│  (Boot → HAL → Sangha → Consciousness → Karma)              │
└─────────────────────────────────────────────────────────────┘
```

---

## Command Categories & Count

### 1. File Operations (80 commands)
**POSIX**: ls, cp, mv, rm, mkdir, rmdir, touch, cat, more, less, head, tail, grep, find, locate, file, stat, df, du, ln, readlink, basename, dirname, mkfifo, dd, cmp, diff, patch, tee, split, csplit, expand, unexpand, fmt, fold, join, paste, pr, tr, uniq, wc

**Windows**: dir, copy, xcopy, robocopy, move, del, erase, rd, rmdir, mkdir, md, type, more, find, findstr, comp, fc, tree, attrib, compact, cipher, where

**macOS**: Same as POSIX + mdls, mdfind, xattr, ditto

### 2. Text Processing (40 commands)
**POSIX**: sed, awk, cut, sort, uniq, tr, wc, grep, egrep, fgrep, strings, od, hexdump, iconv, col, colrm, column, look, rev

**Windows**: sort, find, findstr, more

**macOS**: Same as POSIX + textutil

### 3. Process Management (35 commands)
**POSIX**: ps, top, htop, kill, killall, pkill, pgrep, jobs, bg, fg, nohup, nice, renice, pidof, pstree, watch, timeout, fuser, lsof

**Windows**: tasklist, taskkill, start, taskmgr

**macOS**: Same as POSIX + launchctl, osascript

### 4. Network (45 commands)
**POSIX**: ping, traceroute, netstat, ifconfig, ip, route, arp, nslookup, dig, host, curl, wget, ftp, sftp, scp, ssh, telnet, nc, ncat, socat, tcpdump, nmap, iptables

**Windows**: ping, tracert, netstat, ipconfig, nslookup, route, arp, netsh, curl

**macOS**: Same as POSIX + networksetup

### 5. System Information (30 commands)
**POSIX**: uname, hostname, uptime, whoami, who, w, last, date, cal, env, printenv, set, export, alias, unalias, history, id, groups

**Windows**: ver, hostname, systeminfo, set, echo, time, date, whoami

**macOS**: Same as POSIX + system_profiler, sw_vers

### 6. Permissions & Ownership (15 commands)
**POSIX**: chmod, chown, chgrp, umask, getfacl, setfacl, chattr, lsattr, sudo, su

**Windows**: icacls, takeown, cacls, runas

**macOS**: Same as POSIX

### 7. Compression & Archives (25 commands)
**POSIX**: tar, gzip, gunzip, bzip2, bunzip2, xz, unxz, zip, unzip, compress, uncompress, zcat, bzcat, xzcat, ar, cpio

**Windows**: compact, expand, makecab, extrac32

**macOS**: Same as POSIX + hdiutil, diskutil

### 8. Package Management (20 commands)
**POSIX**: apt, apt-get, dpkg, yum, dnf, rpm, pacman, zypper, emerge

**Windows**: choco, winget, scoop

**macOS**: brew, port, mas

### 9. Disk & Storage (25 commands)
**POSIX**: mount, umount, df, du, fdisk, parted, mkfs, fsck, dd, sync, blkid, lsblk, hdparm

**Windows**: diskpart, chkdsk, format, label, vol, fsutil

**macOS**: Same as POSIX + diskutil, hdiutil

### 10. User Management (15 commands)
**POSIX**: useradd, userdel, usermod, groupadd, groupdel, passwd, chpasswd, finger

**Windows**: net user, net localgroup

**macOS**: dscl, dseditgroup

### 11. Shell Builtins (30 commands)
**Common**: cd, pwd, echo, printf, test, [, read, exit, return, exec, eval, source, ., true, false, yes, no, shift, getopts, break, continue, wait, sleep, clear, reset

### 12. Scripting & Control (20 commands)
**POSIX**: if, then, else, elif, fi, case, esac, for, while, until, do, done, function, select

**Windows**: if, else, for, goto, call, exit

**macOS**: Same as POSIX

### 13. Development Tools (15 commands)
**POSIX**: gcc, make, gdb, strace, ltrace, nm, objdump, readelf, ar, ranlib

**Windows**: cl, msbuild, devenv

**macOS**: clang, xcodebuild, instruments

### 14. Miscellaneous (15 commands)
**POSIX**: xargs, parallel, seq, shuf, factor, base64, base32, md5sum, sha256sum

**Windows**: certutil

**macOS**: pbcopy, pbpaste, open, say

---

## Implementation Strategy

### Phase 1: Core Shell Infrastructure (Week 1)
- Command parser with syntax detection
- Command dispatcher/registry
- Pipe & redirect handling
- Variable expansion
- **Deliverable**: Basic shell loop working

### Phase 2: Filesystem Integration (Week 1)
- PXFS integration (compressed files)
- UCFS integration (universal paths)
- RF2S integration (rock-solid storage)
- Ramdisk integration (memory FS)
- **Deliverable**: All FS commands working

### Phase 3: POSIX Commands - Batch 1 (Week 2)
- File operations (40 commands)
- Text processing (20 commands)
- **Deliverable**: 60 POSIX commands

### Phase 4: POSIX Commands - Batch 2 (Week 2)
- Process management (20 commands)
- System info (15 commands)
- Permissions (10 commands)
- **Deliverable**: 45 more POSIX commands

### Phase 5: Windows Commands (Week 3)
- File operations (25 commands)
- System commands (30 commands)
- Network commands (15 commands)
- **Deliverable**: 70 Windows commands

### Phase 6: macOS Commands (Week 3)
- BSD extensions (20 commands)
- macOS-specific (15 commands)
- **Deliverable**: 35 macOS commands

### Phase 7: Integration & Testing (Week 4)
- Full integration with Boot + HAL + Sangha
- Consciousness-aware command execution
- Karma tracking for all commands
- Comprehensive test suite
- **Deliverable**: Production-ready universal shell

---

## File Structure

```
src/shell/
├── universal/
│   ├── tbos_universal_shell.h          # Main shell header
│   ├── tbos_universal_shell.c          # Shell core
│   ├── command_parser.c                 # Syntax parser
│   ├── command_dispatcher.c             # Command routing
│   ├── command_registry.c               # All 400 commands
│   │
│   ├── posix/
│   │   ├── file_ops.c                   # ls, cp, mv, rm...
│   │   ├── text_processing.c            # sed, awk, grep...
│   │   ├── process_mgmt.c               # ps, kill, top...
│   │   ├── network.c                    # ping, curl, ssh...
│   │   └── ...
│   │
│   ├── windows/
│   │   ├── file_ops.c                   # dir, copy, del...
│   │   ├── system.c                     # tasklist, net...
│   │   └── ...
│   │
│   ├── macos/
│   │   ├── bsd_extensions.c             # macOS-specific
│   │   └── ...
│   │
│   └── filesystem/
│       ├── pxfs_integration.c           # PXFS commands
│       ├── ucfs_integration.c           # UCFS commands
│       ├── rf2s_integration.c           # RF2S commands
│       └── ramdisk_integration.c        # Ramdisk commands
│
tests/shell/
├── test_posix_commands.c
├── test_windows_commands.c
├── test_macos_commands.c
└── test_filesystem_integration.c
```

---

## API Design

### Command Registration

```c
typedef struct {
    const char* name;
    const char* aliases[5];           /* Alternative names */
    const char* description;
    const char* usage;
    int (*handler)(int argc, char** argv);
    uint32_t os_support;              /* POSIX|WINDOWS|MACOS */
    karma_score_t karma_cost;
    karma_score_t karma_reward;
    bool requires_filesystem;
    bool requires_network;
} universal_command_t;
```

### Filesystem Operations

```c
/* Transparent compression with PXFS */
int universal_cat(const char* path);     // Auto-decompress if PXFS
int universal_ls(const char* path);      // Show compression ratios
int universal_cp(const char* src, const char* dst, bool compress);

/* Universal paths with UCFS */
int universal_cd(const char* path);      // Windows/POSIX/macOS paths
int universal_pwd(void);                 // Current format

/* Rock-solid storage with RF2S */
int universal_fsck(const char* device);  // Verify RF2S integrity
int universal_mount(const char* device, const char* mountpoint);
```

---

## Revolutionary Features

1. **Syntax Auto-Detection**:
   - Detects if command is POSIX, Windows, or macOS syntax
   - Automatically translates: `dir` → `ls`, `copy` → `cp`

2. **Cross-Platform Paths**:
   - `/home/user` works on Windows → `C:\Users\user`
   - `C:\Windows` works on Linux → `/mnt/c/Windows`

3. **Universal Compression**:
   - All file operations can use PXFS compression
   - Transparent to user: `cat file.txt` (auto-decompresses)

4. **Consciousness Integration**:
   - Commands earn/cost karma
   - Higher consciousness unlocks advanced commands
   - Mindful computing tracked

5. **Network Sangha**:
   - Distributed shell execution
   - Share command history across sangha
   - Collective decision-making for destructive operations

---

## Success Metrics

- ✅ All 400 commands implemented
- ✅ 100% syntax compatibility (POSIX/Windows/macOS)
- ✅ PXFS/UCFS/RF2S fully integrated
- ✅ Consciousness/Karma tracking
- ✅ < 10MB binary size
- ✅ Works on 2KB calculator to unlimited cloud
- ✅ 95%+ test coverage

---

## ॥ तत् सत् ॥

**Status**: Architecture Complete
**Next**: Begin Phase 1 Implementation
**ETA**: 4 weeks for full implementation

**Revolutionary Achievement #7**: First universal shell spanning all major operating systems with consciousness integration! 🕉️
