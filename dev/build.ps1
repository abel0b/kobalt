$profile = "debug"
$cc = "cl.exe"
$cflags = "-std=c99 -pedantic -Wall -Wextra -Iinclude -DDEBUG"

# $profile=${1:-release}
# cc=$(basename $(which clang || which gcc))
# cflags="-std=c99 -pedantic -Wall -Wextra -Iinclude"
# if [[ "$profile" = debug ]]; then
#     cflags="$cflags -ggdb3 -Ddebug"
# elif [[ "$profile" = release ]]; then
#     cflags="$cflags -O3"
# else
#     echo "Unknown profile $profile"
#     exit
# fi

# [[ ! -z ${CC+x} ]] && cc=$CC
# [[ ! -z ${CFLAGS+x} ]] && cflags=$CFLAGS

Write-Host "profile = $profile"
Write-Host "cc = $cc"
Write-Host "cflags = $cflags"

$target_dir = "target/$profile"
Get-ChildItem "dev" -Filter *.target | Foreach-Object {
    $sources = Get-Content $_.FullName | Select-String -Pattern .target -SimpleMatch
    $command = "$cc $cflags $sources"
    Write-Host $command
}

# for target_conf in dev/*.target; do
#     Write-Host $target_conf
# #     compile="$cc $cflags $(egrep .c$ $target_conf | xargs echo) -o $(basename ${target_conf//.target/})"
# #     echo "$compile"
# #     $compile
# done
