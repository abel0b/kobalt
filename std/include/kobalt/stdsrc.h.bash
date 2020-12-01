echo "unsigned char stdkb[] = {"
cat std/c.kl std/console.kl std/num.kl | xxd -i $file
echo ", 0"
echo "};"
