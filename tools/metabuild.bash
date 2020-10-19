#!/usr/bin/env bash

set -e

# TODO: check timestamps

function metabuild {
    local metasrc=$1
    local target
    if [[ $metasrc =~ .h.bash ]]; then
        target=${metasrc//.h.bash/.h}
    else
        target=${metasrc//.c.bash/.c}
    fi
    local out=$(./$metasrc)
    if [[ $? != 0 ]]; then
        echo "Error in template source"
        exit 1
    fi
    echo "Rule($target $metasrc)"
    echo "$out" > $target
}

function main {
    if [[ -z ${1:x} ]]; then
        while read metasrc; do
            metabuild "$metasrc"
        done < <(find src lib -name "*.bash")
    else
        metabuild $1
    fi
}

main
