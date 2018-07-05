if command -v flatpak > /dev/null; then
    # set XDG_DATA_DIRS to include Flatpak installations

    new_dirs=$(
        (
            unset G_MESSAGES_DEBUG
            echo "${XDG_DATA_HOME:-"$HOME/.local/share"}/flatpak"
            GIO_USE_VFS=local flatpak --installations
        ) | (
            new_dirs=
            while read -r install_path
            do
                share_path=$install_path/exports/share
                case ":$XDG_DATA_DIRS:" in
                    (*":$share_path:"*) :;;
                    (*":$share_path/:"*) :;;
                    (*) new_dirs=${new_dirs:+${new_dirs}:}$share_path;;
                esac
            done
            echo "$new_dirs"
        )
    )

    export XDG_DATA_DIRS
    XDG_DATA_DIRS="${new_dirs:+${new_dirs}:}${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

    # Set PATH to include Flatpak launch scripts

    new_dirs=$(
        (
            echo "${XDG_DATA_HOME:-"$HOME/.local/share"}/flatpak"
            flatpak --installations
        ) | (
            new_dirs=
            while read -r install_path
            do
                bin_path=$install_path/exports/bin
                case ":$PATH:" in
                    (*":$bin_path:"*) :;;
                    (*":$bin_path/:"*) :;;
                    (*) new_dirs=${new_dirs:+${new_dirs}:}$bin_path;;
                esac
            done
            echo "$new_dirs"
        )
    )

    export PATH
    PATH="${PATH:-/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games}${new_dirs:+:${new_dirs}}"
fi
