#!/usr/bin/env bash

PS1="(kbenv) $(bash -i -c 'echo "$PS1"')"
PATH="$PATH:build/${1:-release}" bash --rcfile <(cat ~/.bashrc; echo "PS1='$PS1'")
