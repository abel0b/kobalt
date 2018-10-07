add_executable(
    kbc
    main.c
    token.c
    options.c
    lexer.c
    parser.c
    source_file.c
)

target_link_libraries(kbc array)
