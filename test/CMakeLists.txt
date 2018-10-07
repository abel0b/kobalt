add_executable(test_array main.c test_array.c)
target_link_libraries(test_array utest array)
add_custom_target(
    unit-test
    COMMAND $<TARGET_FILE:test_array>
    DEPENDS test_array
)
add_custom_target(
    memcheck
    COMMAND valgrind $<TARGET_FILE:test_array>
    DEPENDS test_array
)
