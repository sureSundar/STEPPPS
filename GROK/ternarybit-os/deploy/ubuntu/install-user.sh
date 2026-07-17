#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
hosted_dir="$repo_root/deploy/hosted"
prefix="${XDG_DATA_HOME:-$HOME/.local/share}/tbos"
bin_dir="$HOME/.local/bin"
app_dir="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
autostart_dir="${XDG_CONFIG_HOME:-$HOME/.config}/autostart"
config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/tbos"
enable_autostart=0

case "${1:-}" in
    "") ;;
    --autostart) enable_autostart=1 ;;
    *) echo "usage: $0 [--autostart]" >&2; exit 64 ;;
esac

if ! command -v pkg-config >/dev/null 2>&1 || ! pkg-config --exists sdl2; then
    echo "SDL2 development files are required." >&2
    echo "Install them with: sudo apt-get install build-essential pkg-config libsdl2-dev" >&2
    exit 69
fi

make -C "$hosted_dir" desktop

mkdir -p "$prefix/bin" "$bin_dir" "$app_dir" "$config_dir"
install -m755 "$hosted_dir/build/tbos-desktop" "$prefix/bin/tbos-desktop"
install -m755 "$hosted_dir/tbos-desktop-session" "$prefix/bin/tbos-desktop-session"

ln -sfn "$prefix/bin/tbos-desktop-session" "$bin_dir/tbos-desktop-session"

sed "s|Exec=/usr/bin/tbos-desktop-session|Exec=$prefix/bin/tbos-desktop-session|" \
    "$hosted_dir/tbos-desktop.desktop" > "$app_dir/tbos-desktop.desktop"

cat > "$config_dir/hosted.conf" <<EOF
TBOS_PROFILE=ubuntu-full
TBOS_DESKTOP_BIN=$prefix/bin/tbos-desktop
TBOS_HOST_OS=ubuntu
EOF

if [ "$enable_autostart" -eq 1 ]; then
    mkdir -p "$autostart_dir"
    sed "s|Exec=/usr/bin/tbos-desktop-session|Exec=$prefix/bin/tbos-desktop-session|" \
        "$hosted_dir/tbos-desktop.desktop" > "$autostart_dir/tbos-desktop.desktop"
    printf '\nX-GNOME-Autostart-enabled=true\n' >> "$autostart_dir/tbos-desktop.desktop"
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$app_dir" >/dev/null 2>&1 || true
fi

echo "TBOS Ubuntu profile installed for $USER."
echo "Launch: $prefix/bin/tbos-desktop-session"
if [ "$enable_autostart" -eq 0 ]; then
    echo "Autostart is disabled. Re-run with --autostart to enable it."
fi
