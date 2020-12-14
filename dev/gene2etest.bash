#!/bin/bash

set -e

cp $1 test/e2e

name=$(basename $1)

./bin/debug/kobalt -n -L $1 > test/e2e/${name}tok

./bin/debug/kobalt -n -P $1 > test/e2e/${name}ast

./bin/debug/kobalt -n -T $1 > test/e2e/${name}tast

./bin/debug/kobalt -n $1
echo $? > test/e2e/${name}code
