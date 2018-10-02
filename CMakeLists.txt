cmake_minimum_required(VERSION 3.1)

include_directories(include)

set(CFLAGS "${CFLAGS} -fprofile-instr-generate -fcoverage-mapping -g")

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/array)
add_subdirectory(lib/test)
