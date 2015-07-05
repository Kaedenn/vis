#!/bin/bash

which ffmpeg || {
    echo "please install the libffmpeg version of ffmpeg" >&2
    echo '(and NOT the libav version!)' >&2
    exit 1
}

i_img="$1"
i_wav="$2"
o_avi="$3"

ffmpeg -framerate 30 -i "$i_img" -i "$i_wav" -shortest -c:v libx264 "$o_avi"
