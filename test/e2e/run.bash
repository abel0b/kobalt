#!/usr/bin/env bash

for program in test/e2e/*.kb
do
    echo -n ">> $program "
    temp=$(mktemp)
    command="./build/test/kbc -L $program"
    $command > $temp 2>&1
    ref=${program//.kb/.kb-L}
    if [[ $? == 0 ]]
    then
        if ! cmp -s $ref $temp
        then
            echo -e "\e[41m FAIL \e[0m"
            if [[ ! -z ${VERBOSE+x} ]]
            then
                diff --color $ref $temp
                echo "$command"
                exit 1
            fi
            continue
        fi
    else
        echo -e "\e[41m FAIL \[0m runtime error"
        if [[ ! -z ${VERBOSE+x} ]]
        then
            cat $temp
            echo "$command"
            exit 1
        fi
        continue
    fi
    cmdvg="valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1"
    $cmdvg $command > $temp 2>&1
    if [[ $? == 0 ]]
    then
        echo -e "\e[42m PASS \e[0m"
    else
        echo -e "\e[41m FAIL \e[0m valgrind error"
        if [[ ! -z ${VERBOSE+x} ]]
        then
            cat $temp
            echo $cmdvg $command
            exit 1
        fi
    fi
done
