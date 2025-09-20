#!/usr/bin/env python3
import argparse
import json
import os
import sys
import difflib
import math

SCHEMA_DIR = os.path.join(os.path.dirname(__file__), '..', 'schemas')
FRAME_SCHEMA = os.path.join(SCHEMA_DIR, 'steppps_frame.json')
EVENT_SCHEMA = os.path.join(SCHEMA_DIR, 'steppps_event.json')
PROMPT_SCHEMA = os.path.join(SCHEMA_DIR, 'steppps_prompt.json')


def load_json(path_or_dash: str):
    if path_or_dash == '-' or not path_or_dash:
        return json.load(sys.stdin)
    with open(path_or_dash) as f:
        return json.load(f)


def dumps(obj):
    return json.dumps(obj, indent=2, sort_keys=True)


def try_jsonschema_validate(kind: str, obj: dict) -> (bool, str):
    try:
        import jsonschema
    except Exception:
        # fallback: minimal checks
        if kind == 'frame':
            ok = isinstance(obj, dict) and 'meta' in obj and 'time' in obj
            return ok, 'jsonschema not available; performed minimal checks'
        if kind == 'event':
            ok = isinstance(obj, dict) and 'time' in obj and 'kind' in obj
            return ok, 'jsonschema not available; performed minimal checks'
        if kind == 'prompt':
            ok = isinstance(obj, dict) and 'meta' in obj and 'prompt' in obj
            return ok, 'jsonschema not available; performed minimal checks'
        return False, 'unknown kind'
    schema_path = {'frame': FRAME_SCHEMA, 'event': EVENT_SCHEMA, 'prompt': PROMPT_SCHEMA}.get(kind)
    if not schema_path or not os.path.exists(schema_path):
        return False, f'schema not found for {kind}'
    with open(schema_path) as f:
        schema = json.load(f)
    try:
        jsonschema.validate(instance=obj, schema=schema)
        return True, 'valid'
    except Exception as e:
        return False, str(e)


def cmd_validate(args):
    obj = load_json(args.file)
    ok, msg = try_jsonschema_validate(args.kind, obj)
    print(('OK ' if ok else 'ERR ') + msg)
    sys.exit(0 if ok else 1)


def cmd_render(args):
    obj = load_json(args.file)
    # produce a simple summary
    meta = obj.get('meta', {})
    time = obj.get('time', {})
    print(f"version={meta.get('version','?')} id={meta.get('id','?')} profile={meta.get('profile','?')} ts={time.get('ts','?')}")
    if 'events' in obj:
        print(f"events={len(obj.get('events', []))}")
    if 'links' in obj:
        print(f"links={len(obj.get('links', []))}")


def cmd_diff(args):
    a = dumps(load_json(args.old)).splitlines(keepends=True)
    b = dumps(load_json(args.new)).splitlines(keepends=True)
    for line in difflib.unified_diff(a, b, fromfile=args.old, tofile=args.new):
        sys.stdout.write(line)


def cmd_transform(args):
    obj = load_json(args.file)
    # Helper: load display profile
    def load_display_profile(path: str|None):
        prof_path = path or '/etc/tbos/display-profile.json'
        try:
            with open(prof_path) as f:
                return json.load(f)
        except Exception:
            # default peaks (nm) for sRGB-ish panel and gamma 2.2
            return {
                'gamma': 2.2,
                'primaries_nm': {'R': 610.0, 'G': 540.0, 'B': 460.0},
                'sigma_nm': 15.0
            }
    if args.op == 'pixel.replace_color':
        from_col = tuple(map(int, args.from_color.split(',')))
        to_col = tuple(map(int, args.to_color.split(',')))
        # naive: replace in obj['pixel']['data'] if present as list of [r,g,b]
        pixel = obj.get('pixel', {})
        data = pixel.get('data')
        if isinstance(data, list):
            out = []
            for p in data:
                if isinstance(p, list) and len(p) == 3 and tuple(p) == from_col:
                    out.append(list(to_col))
                else:
                    out.append(p)
            pixel['data'] = out
            obj['pixel'] = pixel
        print(dumps(obj))
        return
    if args.op == 'spectrum.features':
        # args.file may be pf2s://<lambda>/spectrum.json or local path
        def pf2s_normalize_nm(val: float) -> str:
            return f"{val:.6f}"
        def pf2s_resolve(vpath: str) -> str:
            if vpath.startswith('pf2s://'):
                rest = vpath[len('pf2s://'):]
                parts = rest.split('/', 1)
                lam = pf2s_normalize_nm(float(parts[0]))
                name = parts[1] if len(parts) > 1 else 'spectrum.json'
                mp = os.path.join('/mnt/pf', lam)
                bp = os.path.join('/var/lib/pf2s', lam)
                base = mp if os.path.exists(mp) else bp
                return os.path.join(base, name)
            return vpath
        path = pf2s_resolve(args.file)
        with open(path) as f:
            spec = json.load(f)
        samples = spec.get('samples') or []
        xs = [s.get('lambda_nm') for s in samples if 'lambda_nm' in s]
        ys = [s.get('power_dbm') for s in samples if 'power_dbm' in s]
        # Detect up to N prominent peaks (local minima if spectrum is in dBm where dips are peaks); make it configurable
        N = int(getattr(args, 'max_peaks', 5))
        window = int(getattr(args, 'window', 3))
        mode = getattr(args, 'mode', 'minima')  # 'minima' or 'maxima'
        peaks = []
        for i in range(window, len(xs)-window):
            seg = ys[i-window:i+window+1]
            y = ys[i]
            if mode == 'minima':
                if y == min(seg):
                    peaks.append((xs[i], y))
            else:
                if y == max(seg):
                    peaks.append((xs[i], y))
        # sort by prominence
        if mode == 'minima':
            peaks.sort(key=lambda p: p[1])  # more negative first
        else:
            peaks.sort(key=lambda p: -p[1])
        peaks = peaks[:N]
        out = {
            'range_nm': spec.get('range_nm'),
            'points': spec.get('points'),
            'peaks': [{'lambda_nm': round(l, 4), 'power_dbm': round(v, 3)} for (l,v) in peaks]
        }
        print(dumps(out))
        return
    if args.op == 'wave.fft2':
        # Educational 2D DFT magnitude for a small grayscale matrix.
        # Input JSON may be:
        #   {"pixel":{"gray": [[..],[..],...]}} OR
        #   {"pixel":{"data":[[r,g,b],...]}} where length is a perfect square; we convert to grayscale row-major.
        # Output: {"width":W,"height":H,"topk":[{"u":i,"v":j,"mag":...}],"center_mag":...}
        def get_gray_matrix(o):
            if isinstance(o, dict):
                p = o.get('pixel', {})
                if isinstance(p, dict) and 'gray' in p:
                    return p.get('gray')
                d = p.get('data') if isinstance(p, dict) else None
                if isinstance(d, list) and d:
                    # assume row-major perfect square; convert to grayscale
                    import math as _m
                    n = len(d)
                    s = int(_m.isqrt(n))
                    if s*s == n:
                        rows = []
                        for r in range(s):
                            row = []
                            for c in range(s):
                                idx = r*s + c
                                pix = d[idx]
                                if isinstance(pix, list) and len(pix) == 3:
                                    # simple luma
                                    row.append(0.2126*pix[0] + 0.7152*pix[1] + 0.0722*pix[2])
                                else:
                                    row.append(0.0)
                            rows.append(row)
                        return rows
            return None
        M = get_gray_matrix(obj)
        if not M:
            print(dumps({"error":"no suitable gray matrix"}))
            return
        H = len(M); W = len(M[0]) if H>0 else 0
        # Naive DFT O(N^3) for clarity; okay for small sizes
        import cmath
        F = [[0.0 for _ in range(W)] for __ in range(H)]
        for u in range(H):
            for v in range(W):
                s = 0+0j
                for x in range(H):
                    for y in range(W):
                        angle = -2.0*math.pi*((u*x)/H + (v*y)/W)
                        s += complex(M[x][y],0.0) * complex(math.cos(angle), math.sin(angle))
                F[u][v] = abs(s)
        # Collect top-k excluding DC (0,0)
        topk_n = int(getattr(args,'topk',10))
        entries = []
        for u in range(H):
            for v in range(W):
                if u==0 and v==0:
                    continue
                entries.append((F[u][v], u, v))
        entries.sort(reverse=True)
        top = [{"u":u,"v":v,"mag":round(m,3)} for (m,u,v) in entries[:topk_n]]
        out = {"width":W, "height":H, "topk": top, "center_mag": round(F[0][0],3)}
        print(dumps(out))
        return
    if args.op == 'spectrum.from_pixel':
        # Map pixel RGB (0-255 or 0-1) to synthetic spectrum using Gaussian SPDs for primaries
        prof = load_display_profile(getattr(args, 'profile', None))
        gamma = float(prof.get('gamma', 2.2))
        peaks = prof.get('primaries_nm', {'R':610.0,'G':540.0,'B':460.0})
        sigma = float(prof.get('sigma_nm', 15.0))
        # Extract a single RGB triple; support multiple shapes
        def get_rgb(o):
            # Try frame.pixel.rgb
            if isinstance(o, dict):
                p = o.get('pixel', {})
                if isinstance(p, dict) and 'rgb' in p:
                    return p.get('rgb')
                # or pixel.data as first element
                d = p.get('data') if isinstance(p, dict) else None
                if isinstance(d, list) and d:
                    if isinstance(d[0], list) and len(d[0]) == 3:
                        return d[0]
            # Try root rgb
            if isinstance(o, dict) and 'rgb' in o:
                return o.get('rgb')
            return [255,255,255]
        rgb = get_rgb(obj)
        # normalize to 0..1 and linearize
        def lin(c):
            v = float(c)
            if v > 1.0:
                v = v/255.0
            return max(0.0, min(1.0, v)) ** gamma  # inverse EOTF approx
        Rw, Gw, Bw = lin(rgb[0]), lin(rgb[1]), lin(rgb[2])
        # Build spectrum on a reasonable nm grid (380..780nm)
        start, end, step = 380.0, 780.0, 2.0
        xs = []
        ys = []
        def gauss(x, mu, sig):
            return math.exp(-0.5 * ((x-mu)/sig)**2)
        nmR, nmG, nmB = float(peaks['R']), float(peaks['G']), float(peaks['B'])
        for i in range(int((end-start)/step)+1):
            x = start + i*step
            xs.append(round(x, 4))
            y = 0.0
            y += Rw * gauss(x, nmR, sigma)
            y += Gw * gauss(x, nmG, sigma)
            y += Bw * gauss(x, nmB, sigma)
            ys.append(y)
        # Normalize to dBm-ish relative units
        ymax = max(ys) if ys else 1.0
        samples = []
        for i, x in enumerate(xs):
            pwr = -60.0 + 20.0*math.log10((ys[i]/ymax) + 1e-6)
            samples.append({'lambda_nm': x, 'power_dbm': round(pwr, 3)})
        out = {'range_nm': [start, end], 'points': len(samples), 'samples': samples}
        print(dumps(out))
        return
    if args.op == 'pixel.from_spectrum':
        # Approximate an RGB that best matches a given spectrum for this panel profile
        prof = load_display_profile(getattr(args, 'profile', None))
        gamma = float(prof.get('gamma', 2.2))
        peaks = prof.get('primaries_nm', {'R':610.0,'G':540.0,'B':460.0})
        sigma = float(prof.get('sigma_nm', 15.0))
        def pf2s_normalize_nm(val: float) -> str:
            return f"{val:.6f}"
        def pf2s_resolve(vpath: str) -> str:
            if vpath.startswith('pf2s://'):
                rest = vpath[len('pf2s://'):]
                parts = rest.split('/', 1)
                lam = pf2s_normalize_nm(float(parts[0]))
                name = parts[1] if len(parts) > 1 else 'spectrum.json'
                mp = os.path.join('/mnt/pf', lam)
                bp = os.path.join('/var/lib/pf2s', lam)
                base = mp if os.path.exists(mp) else bp
                return os.path.join(base, name)
            return vpath
        path = pf2s_resolve(args.file)
        with open(path) as f:
            spec = json.load(f)
        samples = spec.get('samples') or []
        xs = [s.get('lambda_nm') for s in samples if 'lambda_nm' in s]
        ys = [s.get('power_dbm') for s in samples if 'power_dbm' in s]
        # Convert dBm-like back to linear relative power
        lin_y = [10**((y+60.0)/20.0) for y in ys]
        nmR, nmG, nmB = float(peaks['R']), float(peaks['G']), float(peaks['B'])
        def gauss(x, mu, sig):
            return math.exp(-0.5 * ((x-mu)/sig)**2)
        # Overlap integrals
        wR = sum(lin_y[i] * gauss(xs[i], nmR, sigma) for i in range(len(xs)))
        wG = sum(lin_y[i] * gauss(xs[i], nmG, sigma) for i in range(len(xs)))
        wB = sum(lin_y[i] * gauss(xs[i], nmB, sigma) for i in range(len(xs)))
        # Normalize and apply forward gamma (approx display EOTF inverse)
        m = max(wR, wG, wB, 1e-9)
        Rl, Gl, Bl = wR/m, wG/m, wB/m
        def enc(lin):
            v = max(0.0, min(1.0, lin)) ** (1.0/gamma)
            return int(round(v*255))
        rgb = [enc(Rl), enc(Gl), enc(Bl)]
        out = {'rgb': rgb, 'profile_used': getattr(args, 'profile', None) or '/etc/tbos/display-profile.json'}
        if getattr(args, 'clip', 'warn') == 'warn' and m < (wR+wG+wB)/3.0:
            out['note'] = 'gamut clipping likely'
        print(dumps(out))
        return
    if args.op == 'pixel.threshold':
        # Apply simple threshold on a single channel (R/G/B) with min/max; values outside range set to 0
        channel = args.channel.upper()
        cidx = {'R':0,'G':1,'B':2}.get(channel, 0)
        tmin = int(args.tmin)
        tmax = int(args.tmax)
        pixel = obj.get('pixel', {})
        data = pixel.get('data')
        if isinstance(data, list):
            out = []
            for p in data:
                if isinstance(p, list) and len(p) == 3:
                    v = p[cidx]
                    keep = (v >= tmin and v <= tmax)
                    out.append([p[0], p[1], p[2]] if keep else [0,0,0])
                else:
                    out.append(p)
            pixel['data'] = out
            obj['pixel'] = pixel
        print(dumps(obj))
        return
    if args.op == 'pixel.mask':
        # Keep only pixels equal to mask_color; others set to [0,0,0]
        mask_col = tuple(map(int, args.mask_color.split(',')))
        pixel = obj.get('pixel', {})
        data = pixel.get('data')
        if isinstance(data, list):
            out = []
            for p in data:
                if isinstance(p, list) and len(p) == 3:
                    out.append(p if tuple(p) == mask_col else [0,0,0])
                else:
                    out.append(p)
            pixel['data'] = out
            obj['pixel'] = pixel
        print(dumps(obj))
        return
    # default passthrough
    print(dumps(obj))


def main():
    ap = argparse.ArgumentParser(prog='steppps-tools', description='STEPPPS Tools CLI')
    sub = ap.add_subparsers(dest='cmd')

    ap_v = sub.add_parser('validate', help='Validate steppps JSON')
    ap_v.add_argument('kind', choices=['frame','event','prompt'])
    ap_v.add_argument('file', help='path or - for stdin')
    ap_v.set_defaults(func=cmd_validate)

    ap_r = sub.add_parser('render', help='Summary render')
    ap_r.add_argument('file')
    ap_r.set_defaults(func=cmd_render)

    ap_d = sub.add_parser('diff', help='Semantic-ish diff')
    ap_d.add_argument('old')
    ap_d.add_argument('new')
    ap_d.set_defaults(func=cmd_diff)

    ap_t = sub.add_parser('transform', help='Frame transform ops')
    ap_t.add_argument('op', choices=['pixel.replace_color','pixel.threshold','pixel.mask','spectrum.features','spectrum.from_pixel','pixel.from_spectrum','wave.fft2'])
    ap_t.add_argument('file')
    ap_t.add_argument('--from-color', default='0,0,255')
    ap_t.add_argument('--to-color', default='255,64,0')
    ap_t.add_argument('--channel', default='R', help='R/G/B for threshold')
    ap_t.add_argument('--tmin', default='0')
    ap_t.add_argument('--tmax', default='255')
    ap_t.add_argument('--mask-color', default='255,255,255')
    ap_t.add_argument('--max-peaks', default='5')
    ap_t.add_argument('--window', default='3')
    ap_t.add_argument('--mode', default='minima')
    ap_t.add_argument('--profile', help='Display profile JSON (primaries/gamma)')
    ap_t.add_argument('--clip', default='warn', help='pixel.from_spectrum gamut handling')
    ap_t.add_argument('--topk', default='10', help='wave.fft2 top-k components')
    ap_t.set_defaults(func=cmd_transform)

    args = ap.parse_args()
    if not args.cmd:
        ap.print_help(); sys.exit(1)
    args.func(args)

if __name__ == '__main__':
    main()
