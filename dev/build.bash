#!/bin/bash

set -e

profile=${1:-release}
cc=$(basename $(which clang || which gcc))
cflags="-std=c99 -pedantic -Wall -Wextra -Iinclude"
if [[ "$profile" = release || "$profile" = test ]]; then
    cflags="$cflags -O3"
elif [[ "$profile" = debug ]]; then
    cflags="$cflags -ggdb3 -DDEBUG=1"
else
    echo "Unknown profile $profile"
    exit
fi
cflags="$cflags -DKBVERSION=\"0.0.0\""

[[ ! -z ${CC+x} ]] && cc=$CC
[[ ! -z ${CFLAGS+x} ]] && cflags=$CFLAGS

echo "profile = $profile"
echo "cc = $cc"
echo "cflags = $cflags"

distdir="dist/$profile"
mkdir -p "$distdir"
for target in dev/*.binary; do
    compile="$cc $cflags $(egrep .c$ $target | xargs echo) -o $distdir/$(basename ${target//.binary/})"
    echo "$compile"
    $compile
done
