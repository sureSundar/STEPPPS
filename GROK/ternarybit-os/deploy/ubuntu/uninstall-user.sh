#!/bin/sh
set -eu

prefix="${XDG_DATA_HOME:-$HOME/.local/share}/tbos"
bin_link="$HOME/.local/bin/tbos-desktop-session"
app_file="${XDG_DATA_HOME:-$HOME/.local/share}/applications/tbos-desktop.desktop"
autostart_file="${XDG_CONFIG_HOME:-$HOME/.config}/autostart/tbos-desktop.desktop"
config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/tbos"

if [ -L "$bin_link" ]; then
    unlink "$bin_link"
fi

rm -f "$app_file" "$autostart_file"
rm -rf "$prefix" "$config_dir"

echo "TBOS Ubuntu user profile removed. Ubuntu itself was not modified."
