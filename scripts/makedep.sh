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
        if (system("test "f" -nt .d/"d) == 0) {
            print f ":" incs[f] > ".d/"d
        }
    }
}
AWK_SCRIPT

prep_depfile() {
    if [[ $1 -nt $2 ]]; then
        echo "Purging $1's depfile: $2"
        rm "$2"
    else
        echo "Keeping $1's depfile: $2"
    fi
}

srcs="$(ls -1 "$SRCDIR" | \grep '\.c$')"
hdrs="$(ls -1 "$SRCDIR" | \grep '\.h$')"

for srcf in $srcs; do
    prep_depfile "$srcf" "$SRCDIR/.d/${srcf/.c$/.d}"
done

pushd $SRCDIR >/dev/null
grep -F '#include "' *.h *.c | \
    sed -e 's/\.[hc][^ ]*/.c/' -e 's/#include "/ /g' -e 's/"//g' | \
    while read file; do
        srcf="$(echo "$file" | awk '{ print $1 }')"
        hdrf="$(echo "$file" | awk '{ print $2 }')"
        depf="$(echo "$srcf" | awk '{ sub(/.c$/, ".d"); print }')"
        echo "$depf: $srcf depends on $hdrf"
        if [[ $file -nt ".d/${file/.c/.d}" ]]; then
            echo "Generating $file"
        else
            echo "Preserving $file"
        fi
    done | \
    sort
popd >/dev/null
