#!/usr/bin/env bash

set -e

profile=${1:-release}
cflags="-Isrc/include"
if [[ -z "$cc" ]]; then 
    cflags="$cflags -std=c99 -pedantic -Wall -Wextra -Wshadow"
    if command -v clang; then
        cc=clang
        cflags="$cflags -ferror-limit=2"
    else
        cc=gcc
    fi
    if [[ "$profile" = debug-vg ]]; then
        cflags="$cflags -ggdb3"
    elif [[ "$profile" = debug ]]; then
        cflags="$cflags -ggdb3 -fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls"
    fi
fi

if [[ "$profile" = release || "$profile" = test ]]; then
    cflags="$cflags -O3"
elif [[ "$profile" =~ debug ]]; then
    cflags="-DDEBUG=1 $cflags"
else
    echo "Unknown profile $profile"
    exit
fi

[[ ! -z ${CC+x} ]] && cc=$CC
[[ ! -z ${CFLAGS+x} ]] && cflags=$CFLAGS

echo "profile = $profile"
echo "cc = $cc"
echo "cflags = $cflags"

builddir="build/$profile"
mkdir -p "$builddir"
for target in dev/*.binary; do
    compile="$cc $cflags $(egrep .c$ $target | xargs echo) -o $builddir/$(basename ${target//.binary/})"
    echo "$compile"
    $compile
done
