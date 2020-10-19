#!/usr/bin/env bash

tot=0
nok=0

for program in test/e2e/*.kb; do
    reftok=${program//.kb/.kbtok}
    if [[ -f "$reftok" ]]; then
        filetok=$(mktemp)
        outtok=$(mktemp)
        command="./bin/debug/kobalt -L $program -o $filetok"
        $command > $outtok 2>&1
        exittok=$?
        tot=$(($tot+1))
        if [[ $exittok == 0 ]]; then
            if ! cmp -s $reftok $filetok; then
                nok=$(($nok+1))
                echo -e ">> $program lex \e[41m FAIL \e[0m"
                if [[ ! -z ${VERBOSE+x} ]]; then
                    diff --color $reftok $filetok
                    echo "$command"
                    exit 1
                fi
            fi
        else
            nok=$(($nok+1))
            echo -e ">> $program lex \e[41m FAIL \e[0m runtime error"
            if [[ ! -z ${VERBOSE+x} ]]; then
                cat $outtok
                echo "$command"
                exit 1
            fi
        fi
    fi

    refast=${program//.kb/.kbast}
    if [[ -f "$refast" ]]; then
        fileast=$(mktemp)
        outast=$(mktemp)
        command="./bin/debug/kobalt -T $program -o $fileast"
        $command > $outast 2>&1
        exitast=$?
        tot=$(($tot+1))
        if [[ $exitast == 0 ]]; then
            if ! cmp -s $refast $fileast; then
                nok=$(($nok+1))
                echo -e ">> $program parse \e[41m FAIL \e[0m"
                if [[ ! -z ${VERBOSE+x} ]]; then
                    diff --color $refast $fileast
                    echo "$command"
                    exit 1
                fi
            fi
        else
            nok=$(($nok+1))
            echo -e ">> $program parse \e[41m FAIL \e[0m runtime error"
            if [[ ! -z ${VERBOSE+x} ]]; then
                cat $outast
                echo "$command"
                exit 1
            fi
        fi
    fi

    refstdout=${program//.kb/.stdout}
    if [[ -f "$refstdout" ]]; then
        fileexe=$(mktemp)
        chmod u+x $fileexe
        out=$(mktemp)
        filestdout=$(mktemp)
        command="./bin/debug/kobalt $program -o tmpexe"
        $command > $out 2>&1
        exitstatus=$?
        ./tmpexe > $filestdout
        rm tmpexe
        tot=$(($tot+1))
        if [[ $exitstatus == 0 ]]; then
            if ! cmp -s $refstdout $filestdout; then
                nok=$(($nok+1))
                echo -e ">> $program stdout \e[41m FAIL \e[0m"
                if [[ ! -z ${VERBOSE+x} ]]; then
                    diff --color $refstdout $yu
                    echo "$command"
                    exit 1
                fi
            fi
        else
            nok=$(($nok+1))
            echo -e ">> $program stdout \e[41m FAIL \e[0m runtime error"
            if [[ ! -z ${VERBOSE+x} ]]; then
                cat $out
                echo "$command"
                exit 1
            fi
        fi
    fi
done

if [[ "$nok" = 0 ]]; then
    echo -e "\e[32mall tests passed successfully\e[0m"
else
    echo -e "\e[31m$nok tests out of $tot failed\e[0m"
fi
