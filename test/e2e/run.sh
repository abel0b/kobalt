set -xe
for program in test/e2e/*.kb
do
    ./target/test/kbc "$program" | cmp ${program//.kb/.tokens} -
done
