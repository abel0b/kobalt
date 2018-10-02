add_executable(
    kbc
    main.c
    lexer.c
    token.c
    options.c
    source_file.c
)

target_link_libraries(kbc array)
