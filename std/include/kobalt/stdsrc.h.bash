echo "unsigned char stdkb[] = {"
cat std/c.kb std/console.kb std/num.kb | xxd -i $file
echo "};"
