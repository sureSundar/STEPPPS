#!/usr/bin/env python3
import argparse
import json
import os
import subprocess
import sys

PROFILES_DIR = "/etc/tbos/profiles"
FEATURES_CONF = "/etc/tbos/features.conf"
CGROUP_ROOT = "/sys/fs/cgroup"
RUN_DIR = "/run/tbos"


def ensure_dirs():
    os.makedirs(PROFILES_DIR, exist_ok=True)
    os.makedirs(RUN_DIR, exist_ok=True)


def load_profile(name):
    p = os.path.join(PROFILES_DIR, f"{name}.json")
    with open(p) as f:
        return json.load(f)


def list_profiles():
    ensure_dirs()
    items = []
    for fn in sorted(os.listdir(PROFILES_DIR)):
        if fn.endswith('.json'):
            items.append(fn[:-5])
    return items


def apply_cgroups(name, prof):
    # Require cgroup v2 unified
    if not os.path.exists(os.path.join(CGROUP_ROOT, 'cgroup.controllers')):
        return
    tbos_grp = os.path.join(CGROUP_ROOT, 'tbos')
    grp = os.path.join(tbos_grp, name)
    os.makedirs(grp, exist_ok=True)
    # CPU quota
    cpu = prof.get('cpu', {})
    if 'quota_pct' in cpu:
        pct = max(1, min(100, int(cpu['quota_pct'])))
        # cgroup v2: cpu.max = <max> <period>; use period 100000 and max scaled
        period = 100000
        maxv = int(period * pct / 100)
        try:
            with open(os.path.join(grp, 'cpu.max'), 'w') as f:
                f.write(f"{maxv} {period}")
        except Exception:
            pass
    # Memory limit
    mem = prof.get('memory', {})
    if 'limit_mb' in mem:
        limit = int(mem['limit_mb']) * 1024 * 1024
        try:
            with open(os.path.join(grp, 'memory.max'), 'w') as f:
                f.write(str(limit))
        except Exception:
            pass
    # Move current shell/session into cgroup
    try:
        with open(os.path.join(grp, 'cgroup.procs'), 'w') as f:
            f.write(str(os.getpid()))
    except Exception:
        pass


def apply_display(prof):
    disp = prof.get('display', {})
    cols = disp.get('cols'); rows = disp.get('rows')
    if cols and rows:
        # Best effort: stty size requires rows cols
        try:
            subprocess.run(['stty', 'rows', str(rows), 'cols', str(cols)], check=False)
        except Exception:
            pass


def apply_shell_persona(prof):
    sh = prof.get('shell', {})
    persona = sh.get('persona')
    if persona:
        # Write a hint for tbos-shell
        with open(os.path.join(RUN_DIR, 'shell_persona'), 'w') as f:
            f.write(persona)


def update_features(prof):
    feats = prof.get('features', {})
    if not feats:
        return
    # Read current features.conf
    lines = []
    if os.path.exists(FEATURES_CONF):
        with open(FEATURES_CONF) as f:
            lines = f.read().splitlines()
    env = {}
    for ln in lines:
        if '=' in ln:
            k, v = ln.split('=', 1)
            env[k.strip()] = v.strip()
    # Toggle TBOS_SENSORS
    sensors = set((env.get('TBOS_SENSORS', '').strip() or '').split())
    for d in feats.get('disable', []):
        sensors.discard(d)
    for e in feats.get('enable', []):
        sensors.add(e)
    if sensors:
        env['TBOS_SENSORS'] = ' '.join(sorted(sensors))
    # Write back
    with open(FEATURES_CONF, 'w') as f:
        for k, v in env.items():
            f.write(f"{k}={v}\n")
    # Signal reload
    open(os.path.join(RUN_DIR, 'reload'), 'w').close()


def apply_kexec(prof, accept_reboot: bool):
    kx = prof.get('kexec')
    if not kx or not accept_reboot:
        return
    kernel = kx.get('kernel'); initrd = kx.get('initrd'); cmdline = kx.get('cmdline', '')
    if not kernel or not os.path.exists(kernel):
        print('kexec kernel not found', file=sys.stderr)
        return
    args = ['kexec', '-l', kernel]
    if initrd and os.path.exists(initrd):
        args += ['--initrd', initrd]
    if cmdline:
        args += ['--command-line', cmdline]
    subprocess.run(args, check=False)
    subprocess.run(['sync'])
    subprocess.run(['kexec', '-e'], check=False)


def cmd_list(args):
    for name in list_profiles():
        print(name)


def cmd_apply(args):
    ensure_dirs()
    prof = load_profile(args.name)
    apply_cgroups(args.name, prof)
    apply_display(prof)
    apply_shell_persona(prof)
    update_features(prof)
    if args.i_accept_reboot:
        apply_kexec(prof, True)
    print(f"Applied profile: {args.name}")


def main():
    ap = argparse.ArgumentParser(prog='tbos-profile', description='TBOS scale profile tool')
    sub = ap.add_subparsers(dest='cmd')

    p_list = sub.add_parser('list', help='List profiles')
    p_list.set_defaults(func=cmd_list)

    p_apply = sub.add_parser('apply', help='Apply profile')
    p_apply.add_argument('name')
    p_apply.add_argument('--i-accept-reboot', action='store_true')
    p_apply.set_defaults(func=cmd_apply)

    args = ap.parse_args()
    if not args.cmd:
        ap.print_help(); sys.exit(1)
    args.func(args)

if __name__ == '__main__':
    main()
