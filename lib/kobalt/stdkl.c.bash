echo "#include \"kobalt/stdkl.h\""
echo
echo "char stdkl[] = {"
cat lib/std/c.kl lib/std/console.kl lib/std/num.kl | xxd -i $file
echo ", 0"
echo "};"
