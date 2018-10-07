#include "kobalt/lexer.h"
#include "kobalt/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

char * keywords[8] = {
    "int",
    "float",
    "if",
    "then",
    "else",
    "do",
    "while"
};

enum IndentStyle {
    INDENT_SPACE,
    INDENT_TAB
};

struct Array * lex (struct SourceFile * source_file) {
    printf("== lex %s ==\n", source_file->filename);

    struct Array * tokens = array_make();

    int buffer_size = 2;
    char * buffer = malloc (buffer_size * sizeof (char));
    int cursor = 0;

    int indent_level = 0;
    bool newline = true;
    bool indent_tab = true;
    bool first_indent = true;
    int indent_counter = 0;
    int space_indent = -1;
    int line = 1;

    for (int i = 0; i < source_file->length; i++) {
        if (newline && source_file->content[i] != '\t' && source_file->content[i] != ' ') {
            newline = false;

            if (indent_counter) {
                if (first_indent) {
                    space_indent = indent_counter;
                    first_indent = false;
                }
            }

            if (indent_counter == (indent_level + 1) * space_indent) {
                indent_level ++;
                array_push(tokens, token_make(TOKEN_INDENT, NULL));
            }
            else if (indent_counter == (indent_level - 1) * space_indent) {
                indent_level --;
                array_push(tokens, token_make(TOKEN_DEDENT, NULL));
            }
            indent_counter = 0;
        }

        switch (source_file->content[i]) {
            case '\t':
                if (newline) {
                    indent_counter ++;
                }
                break;
            case '=':
                buffer[cursor] = '\0';
                if (cursor)
                    array_push(tokens, token_make(TOKEN_IDENTIFIER, buffer));
                cursor = 0;
                array_push(tokens, token_make(TOKEN_EQUAL, NULL));
                break;
            case '(':
                array_push(tokens, token_make(TOKEN_LPAR, NULL));
                buffer[cursor] = '\0';
                if (cursor)
                    array_push(tokens, token_make(TOKEN_IDENTIFIER, buffer));
                cursor = 0;
                break;
            case ')':
                buffer[cursor] = '\0';
                if (cursor)
                    array_push(tokens, token_make(TOKEN_IDENTIFIER, buffer));
                cursor = 0;
                break;
            case ' ':
                if (newline) {
                    indent_counter ++;
                    if (indent_tab) {
                        indent_tab = false;
                    }
                }
                buffer[cursor] = '\0';
                if (cursor)
                    array_push(tokens, token_make(TOKEN_IDENTIFIER, buffer));
                cursor = 0;
                break;
            case '\n':
                buffer[cursor] = '\0';
                if (cursor)
                    array_push(tokens, token_make(TOKEN_IDENTIFIER, buffer));
                cursor = 0;
                array_push(tokens, token_make(TOKEN_LF, NULL));
                newline = true;
                line ++;
                break;
            default:
                buffer[cursor] = source_file->content[i];
                cursor++;
        }
        if (cursor == buffer_size) {
            buffer_size = 2 * buffer_size;
            buffer = realloc(buffer, buffer_size * sizeof(char));
        }
    }

    while (indent_level-- > 0) {
        array_push(tokens, token_make(TOKEN_DEDENT, NULL));
    }

    return tokens;
}
