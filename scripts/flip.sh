#!/bin/bash

which convert || {
    echo "please install ImageMagick" >&2
    exit 1
}

iprefix="$1"
oprefix="$2"

dorename() {
    iesc="$(echo "$iprefix" | sed 's/\//\\\//g')"
    oesc="$(echo "$oprefix" | sed 's/\//\\\//g')"
    echo "$@" | sed "s/$iesc/$oesc/"
}

progress=0
doprogress() {
    echo -en "Processed $progress files...\r"
    progress=$(($progress+1))
}

find . -wholename "./$iprefix*" -print | while read f; do
    doprogress
    convert -flip $f $(dorename $f) || exit
done
