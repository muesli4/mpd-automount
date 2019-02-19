#!/bin/bash
# workarround for malfunctioning udisks2 storage plugin

# TODO find a way to pick the directory that gets linked, e.g.:
#     pick <root>/Music
#     pick from .music_paths or hidden file music_paths (each will create one link)
#     pick with user interaction

if [ $# -eq 3 ]; then
    LABEL="$2"
    UUID="$3"
    if [ -z "$LABEL" ]; then
        MPD_MOUNT_NAME="$UUID"
    else
        MPD_MOUNT_NAME="${LABEL} ($UUID)"
    fi
    MUSIC_DIR=~/Music
    LINK_PATH="$MUSIC_DIR/$MPD_MOUNT_NAME"

    refresh_library() {
        mpc update "$MPD_MOUNT_NAME" > /dev/null
        # failed, maybe mpd doesn't run yet
        #if [ $? -eq 1 -a $1 -ge 0 ]; then
        #    sleep 1
        #    refresh_library $(($1 - 1))
        #fi
    }

    if [ "$1" == 'add' ]; then
        if [ $# -eq 4 ]; then
            MOUNT_POINT="$4"
            ln -s "$MOUNT_POINT" "$LINK_PATH" && refresh_library 5
        else
            echo "missing mount point argument"
        fi
    elif [ "$1" == 'remove' ]; then

            rm "$LINK_PATH" && refresh_library 5
    fi
else
    echo "usage:"
    echo "    $0 add LABEL UUID MOUNT_POINT | $0 remove LABEL UUID"
fi
