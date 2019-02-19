#!/bin/bash
# workarround for malfunctioning udisks2 storage plugin

# TODO find a way to pick the directory that gets linked, e.g.:
#     pick <root>/Music
#     pick from .music_paths or hidden file music_paths (each will create one link)
#     pick with user interaction

usage() {
    echo "usage:"
    echo "    $0 add LABEL UUID MOUNT_POINT | $0 remove LABEL UUID"
}

if [ $# -ge 3 ]; then
    CONFIG_REL_PATH=mpd-automount/mpd-automount.conf
    FIRST_CONFIG_PATH="$XDG_CONFIG_HOME/$CONFIG_REL_PATH"
    if [ -n "$XDG_CONFIG_HOME" -a -e "$FIRST_CONFIG_PATH" ]; then
        source "$FIRST_CONFIG_PATH"
    else
        SECOND_CONFIG_PATH="$HOME/.config/$CONFIG_REL_PATH"
        if [ -e "$SECOND_CONFIG_PATH" ]; then
            source "$SECOND_CONFIG_PATH"
        else
            init_config() {
                DIR=$(dirname "$1")
                mkdir -p "$DIR"
                echo "Creating directory: $DIR"
                echo "Writing config to $1."
                echo -e "# trailing slash is not necessary\nMPD_LIBRARY_PATH=~/Music\nLINK_SUB_DIRECTORY=." > "$1"
            }
            if [ -n "$XDG_CONFIG_HOME" ]; then
                CONFIG_PATH=$FIRST_CONFIG_PATH
            else
                CONFIG_PATH=$SECOND_CONFIG_PATH
            fi
            init_config "$CONFIG_PATH"
            source "$CONFIG_PATH"
        fi
    fi

    LABEL="$2"
    UUID="$3"
    if [ -z "$LABEL" ]; then
        MPD_MOUNT_NAME="$UUID"
    else
        MPD_MOUNT_NAME="${LABEL} ($UUID)"
    fi
    LINK_PATH="$MPD_LIBRARY_PATH/$LINK_SUB_DIRECTORY/$MPD_MOUNT_NAME"

    refresh_library() {
        MPD_PATH="$LINK_SUB_DIRECTORY/$MPD_MOUNT_NAME"
        MPD_PATH=${MPD_PATH#./}
        mpc update "$MPD_PATH" > /dev/null
        # failed, maybe mpd doesn't run yet
        #if [ $? -eq 1 -a $1 -ge 0 ]; then
        #    sleep 1
        #    refresh_library $(($1 - 1))
        #fi
    }

    if [ "$1" == 'add' ]; then
        if [ $# -eq 4 ]; then
            if [ -h "$LINK_PATH" -a "$(readlink -m \"$LINK_PATH\")" = "$(readlink -m \"$MOUNT_POINT\")" ]; then
                echo "link already exists, doing nothing"
            else
                MOUNT_POINT="$4"
                ln -s "$MOUNT_POINT" "$LINK_PATH" && refresh_library 5 \
                    && echo "added $MOUNT_POINT to MPD database"
            fi
        else
            echo "missing mount point argument"
        fi
    elif [ "$1" == 'remove' ]; then
        if [ -h "$LINK_PATH" ]; then
            rm "$LINK_PATH" && refresh_library 5 \
                && echo "removed $LINK_PATH from MPD database"
        else
            refresh_library 5
            echo "no link present, refreshing library anyway"
        fi
    else
        usage
    fi
else
    usage
fi
