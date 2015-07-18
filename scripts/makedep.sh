#!/bin/bash

# I found this more fun than (re)learning the gcc -M options, and it appears
# to work quite well for my purposes. So, it's here.

if [[ "$(dirname "$0")" != "." ]]; then
    SRCDIR="$(dirname "$0")/.."
else
    SRCDIR="."
fi

cat <<'AWK_SCRIPT' > "$SRCDIR/scripts/makedep.awk"
BEGIN {
    incs["audio.c"] = ""
}

/^.*c /{
    if (incs[$1] !~ /$2/) {
        incs[$1] = incs[$1] " " $2
    }
}

END {
    for (f in incs) {
        d = f
        sub(/.c$/, ".d", d)
        print f ":" incs[f] > ".d/"d
    }
}
AWK_SCRIPT

pushd $SRCDIR >/dev/null
grep -F '#include "' *.h *.c | \
    sed -e 's/\.[hc][^ ]*/.c/' -e 's/#include "/ /g' -e 's/"//g' | \
    awk -f "$SRCDIR/scripts/makedep.awk" | \
    sort
popd >/dev/null
