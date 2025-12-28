#!/bin/bash

# I found this more fun than (re)learning the gcc -M options, and it appears
# to work quite well for my purposes. So, it's here.

if [[ "$(dirname "$0")" != "." ]]; then
    SRCDIR="$(dirname "$0")/.."
else
    SRCDIR="."
fi

AWK_SCRIPT="$SRCDIR/scripts/makedep.awk"

cat <<'AWK_SCRIPT' > "$AWK_SCRIPT"
BEGIN {
    incs["audio.o"] = ""
}

/^.*\.o /{
    if (incs[$1] !~ /$2/) {
        incs[$1] = incs[$1] " " $2
    }
}

END {
    for (f in incs) {
        d = f
        sub(/\.o$/, ".d", d)
        print ".o/" f ":" incs[f] > ".d/"d
    }
}
AWK_SCRIPT

pushd $SRCDIR >/dev/null
if [[ ! -d ".d" ]]; then
    mkdir .d
fi
grep -F '#include "' *.h *.c | \
    sed -e 's/\.[hc][^ ]*/.o/' -e 's/#include "/ /g' -e 's/"//g' | \
    awk -f "$AWK_SCRIPT" | \
    sort
popd >/dev/null

rm "$AWK_SCRIPT"
