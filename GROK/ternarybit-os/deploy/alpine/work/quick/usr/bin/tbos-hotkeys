#!/usr/bin/env python3
import os
import sys
import time
import subprocess

# Minimal hotkey daemon using evdev.
# Maps Ctrl+Alt+<n> to tbos-profile apply <profile>.
# Profiles mapping can be overridden via /etc/tbos/hotkeys.json

MAPPING_DEFAULT = {
    "1": "calculator",
    "2": "8bit",
    "9": "default"
}

CONFIG = "/etc/tbos/hotkeys.json"


def load_mapping():
    try:
        import json
        if os.path.exists(CONFIG):
            with open(CONFIG) as f:
                m = json.load(f)
                if isinstance(m, dict):
                    return m
    except Exception:
        pass
    return MAPPING_DEFAULT


def list_keyboards():
    # Return list of /dev/input/event* that look like keyboards
    devs = []
    for name in os.listdir('/dev/input') if os.path.isdir('/dev/input') else []:
        if not name.startswith('event'):
            continue
        devs.append(os.path.join('/dev/input', name))
    return devs


def run():
    try:
        from evdev import InputDevice, categorize, ecodes
    except Exception:
        print("py3-evdev not available; tbos-hotkeys disabled", file=sys.stderr)
        return

    mapping = load_mapping()
    keyboards = []
    for path in list_keyboards():
        try:
            dev = InputDevice(path)
            keyboards.append(dev)
        except Exception:
            pass

    if not keyboards:
        # Poll until a device appears
        while not keyboards:
            time.sleep(2)
            try:
                for path in list_keyboards():
                    dev = InputDevice(path)
                    keyboards.append(dev)
            except Exception:
                pass

    # Track modifier state
    ctrl_down = False
    alt_down = False

    from select import select

    while True:
        r, _, _ = select(keyboards, [], [], 1.0)
        for dev in r:
            for event in dev.read():
                if event.type != ecodes.EV_KEY:
                    continue
                code = event.code
                val = event.value  # 1=down, 0=up, 2=hold
                if code in (ecodes.KEY_LEFTCTRL, ecodes.KEY_RIGHTCTRL):
                    ctrl_down = (val != 0)
                elif code in (ecodes.KEY_LEFTALT, ecodes.KEY_RIGHTALT):
                    alt_down = (val != 0)
                elif ctrl_down and alt_down and val == 1:
                    # number keys 1..9
                    key_to_digit = {
                        ecodes.KEY_1: '1', ecodes.KEY_2: '2', ecodes.KEY_3: '3',
                        ecodes.KEY_4: '4', ecodes.KEY_5: '5', ecodes.KEY_6: '6',
                        ecodes.KEY_7: '7', ecodes.KEY_8: '8', ecodes.KEY_9: '9',
                        ecodes.KEY_0: '0'
                    }
                    if code in key_to_digit:
                        d = key_to_digit[code]
                        prof = mapping.get(d)
                        if prof:
                            try:
                                subprocess.Popen(['tbos-profile', 'apply', prof])
                            except Exception:
                                pass

if __name__ == '__main__':
    run()
