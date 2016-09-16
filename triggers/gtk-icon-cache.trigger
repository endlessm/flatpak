#!/bin/sh

update_icon_cache="$(command -v gtk-update-icon-cache-3.0 || command -v gtk-update-icon-cache || true)"

if test \( -x "$update_icon_cache" \) -a \( -d $1/exports/share/icons/hicolor \); then
    cp /usr/share/icons/hicolor/index.theme $1/exports/share/icons/hicolor/
    for dir in $1/exports/share/icons/*; do
        if test -f $dir/index.theme; then
            if ! "$update_icon_cache" --quiet $dir; then
                echo "Failed to run $update_icon_cache for $dir"
                exit 1
            fi
        fi
    done
fi
