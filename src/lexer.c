#include "kobalt/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct kblexer kblexer_make() {
    struct kblexer lexer;
    lexer.state = LEXER_NEWLINE;
    lexer.indent_level = 0;
    lexer.newline = 1;
    lexer.indent_tab = 1;
    lexer.first_indent = 1;
    lexer.first_indent_char = 1;
    lexer.use_tabs_indent = 0;
    lexer.indent_counter = 0;
    lexer.space_indent = -1;
    lexer.line = 1;
    lexer.col = 0;
    kblexer_special_init(&lexer);
    kblexer_int_init(&lexer);
    kblexer_float_init(&lexer);
    lexer.buffer_size = 2;
    lexer.buffer = malloc(lexer.buffer_size * sizeof (char));
    lexer.cursor = 0;
    return lexer;
}

void kblexer_destroy(struct kblexer * lexer) {
    free(lexer->buffer);
}

void kblexer_special_init(struct kblexer * lexer) {
    lexer->special_match.matched = ILLEGAL;
    lexer->special_match.num_matched = ILLEGAL+1;
    lexer->special_match.cursor = 0;
    for (int i=0; i<=ILLEGAL; ++i) {
        lexer->special_match.match[i] = 1;
    }
}

void kblexer_special_next(struct kblexer * lexer, char ch) {
    lexer->special_match.matched = ILLEGAL;
    for (int i=0; i<=ILLEGAL; ++i) {
        if (lexer->special_match.match[i]) {
            int speclen = strlen(specials[i]);
            if (speclen <= lexer->special_match.cursor || specials[i][lexer->special_match.cursor] != ch) {
                lexer->special_match.match[i] = 0;
                lexer->special_match.num_matched --;
            }
            else if(speclen == lexer->special_match.cursor+1) {
                lexer->special_match.matched = i;
            }
        }
    }
    lexer->special_match.cursor ++;
}

void kblexer_float_init(struct kblexer * lexer) {
    lexer->float_match.is_float = 1;
    lexer->float_match.has_dot = 0;
    lexer->float_match.first = 1;
}

void kblexer_float_next(struct kblexer * lexer, char ch) {
    if (lexer->float_match.first) {
        lexer->float_match.first = 0;
        if (ch == '-') return;
    }
    if (!lexer->float_match.has_dot && ch == '.') {
        lexer->float_match.has_dot = 1;
        return;
    }
    if(!(ch >= '0' && ch <= '9')) {
        lexer->float_match.is_float = 0;
    }
}

void kblexer_int_init(struct kblexer * lexer) {
    lexer->int_match.is_integer = 1;
    lexer->int_match.first = 1;
}

void kblexer_int_next(struct kblexer * lexer, char ch) {
    if (lexer->int_match.first) {
        lexer->int_match.first = 0;
        if (ch == '-') return;
    }
    if(!(ch >= '0' && ch <= '9')) {
        lexer->int_match.is_integer = 0;
    }
}

void kblexer_push_token(struct kbtoken ** tokens, unsigned int * num_tokens, unsigned int * capacity, struct kbtoken token) {
    if (*num_tokens == *capacity) {
        (*capacity) = (*capacity) * 2;
        *tokens = realloc(*tokens, sizeof(struct kbtoken) * (*capacity));
        assert(*tokens != NULL);
    }
    (*tokens)[*num_tokens] = token;
    (*num_tokens) ++;
    #if DEBUG
    if(getenv("DEBUG_LEXER")) printf("push  %s=%s\n", kbtoken_string(token.kind), (token.value)?token.value:"");
    #endif
}

void kblexer_push_char(struct kblexer * lexer, char ch) {
    if (lexer->cursor == lexer->buffer_size) {
        lexer->buffer_size = 2 * lexer->buffer_size;
        lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
    }
    lexer->buffer[lexer->cursor] = ch;
    lexer->cursor++;
}

void kblexer_buf_init(struct kblexer * lexer) {
    lexer->cursor = 0;
}

char * kblexer_state_str(struct kblexer * lexer) {
    switch(lexer->state) {
        case LEXER_NEWLINE:
            return "NEWLINE";
        case LEXER_NEWTOK:
            return "NEWTOK";
        case LEXER_SYM:
            return "SYM";
        case LEXER_STRINGLIT:
            return "STRINGLIT";
        case LEXER_NUMLIT:
            return "NUMLIT";
        case LEXER_COMMENT:
            return "COMMENT";
        case LEXER_IDENTIFIER:
            return "IDENTIFIER";
    }
    return "UNDEFINED";
}

static int is_stmt(struct kbtoken * tok) {
    return tok->kind != COMMENT;
}

static struct kbtoken * last_token(struct kbtoken ** tokens, unsigned int * num_tokens) {
    return &((*tokens)[(*num_tokens)-1]);
}

void kblexer_next(struct kblexer * lexer, struct kbtoken ** tokens, unsigned int * num_tokens, unsigned int * capacity, char ch) {
    #if DEBUG
    if(getenv("DEBUG_LEXER")) printf("next  cursor=%c state=%s\n", ch, kblexer_state_str(lexer));
    #endif
    switch(lexer->state) {
        case LEXER_NEWLINE:
            if (ch == ' ') {
                if (!lexer->first_indent_char && lexer->use_tabs_indent) {
                    fprintf(stderr, "Inconsistent use of tabs and space in indentation\n");
                    exit(1);
                }
                lexer->first_indent_char = 0;
                lexer->indent_counter ++;
            }
            else if (ch == '\t') {
                if (lexer->first_indent_char) {
                    lexer->use_tabs_indent = 1;
                }
                else if (!lexer->use_tabs_indent) {
                    fprintf(stderr, "Inconsistent use of tabs and space in indentation\n");
                    exit(1);
                }
                lexer->first_indent_char = 0;
                lexer->indent_counter ++;
            }
            else if (ch == '\n') {
                lexer->indent_counter = 0;
            }
            else {
                if (lexer->first_indent) {
                    if (lexer->indent_counter) {
                        lexer->space_indent = (lexer->use_tabs_indent)?1:lexer->indent_counter;
                        lexer->first_indent = 0;
                        // lexer->indent_level++;
                        // kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(INDENT, NULL, lexer->line, 1));
                    }
                    else if(*num_tokens>0 && is_stmt(last_token(tokens, num_tokens))) {
                        kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(SEMI, NULL, lexer->line-1, -1));
                    }
                }
                if (lexer->indent_counter % lexer->space_indent == 0) {
                    if (lexer->indent_counter/lexer->space_indent > lexer->indent_level) {
                        for(int kk=0; kk<(lexer->indent_counter / lexer->space_indent); ++kk)
                            kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(INDENT, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }
                    else if (lexer->indent_counter/lexer->space_indent < lexer->indent_level) {
                        for(int kk=0; kk<(lexer->indent_counter / lexer->space_indent); ++kk)
                            kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(DEDENT, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }
                    else if(lexer->line + lexer->col > 2 && is_stmt(last_token(tokens, num_tokens))) {
                        kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(SEMI, NULL, lexer->line-1, -1));
                    }
                }
                else {
                    fprintf(stderr, "Unexpected indentation at %d:%d\n", lexer->line, lexer->col);
                    exit(1);
                }
                lexer->indent_counter = 0;
                lexer->state = LEXER_NEWTOK;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            break;
        case LEXER_SYM:
            if (is_delim(ch)) {
                kblexer_push_char(lexer, '\0');
                
                if (lexer->special_match.matched == COMMENT) {
                    lexer->state = LEXER_COMMENT;
                }
                else {
                    if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                        kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(lexer->special_match.matched, (lexer->special_match.matched == ILLEGAL)? lexer->buffer : NULL, lexer->tokline, lexer->tokcol));
                    }
                }

                kblexer_buf_init(lexer);
                kblexer_special_init(lexer);
            }
            else if (is_sep(ch)) {
                int prevmatched = lexer->special_match.matched;
                kblexer_special_next(lexer, ch); 
                if (lexer->special_match.num_matched == 0) {
                    if (lexer->cursor > 0) {
                        if (prevmatched == COMMENT) {
                            lexer->state = LEXER_COMMENT;
                        }
                        else {
                            lexer->state = LEXER_NEWTOK;
                            kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(prevmatched, (prevmatched == ILLEGAL)? lexer->buffer : NULL, lexer->line, lexer->col));
                        }       
                        
                        kblexer_special_init(lexer);
                        kblexer_buf_init(lexer);
                        kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                    }
                    else {
                        fprintf(stderr, "Unexpected character at %d:%d\n", lexer->line, lexer->col);
                        exit(1);
                    }
                }
                else if (lexer->special_match.num_matched == 1 && lexer->special_match.matched != ILLEGAL) {
                    if (lexer->special_match.matched == COMMENT) {
                        lexer->state = LEXER_COMMENT;
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                        kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(lexer->special_match.matched, NULL, lexer->line, lexer->col));
                    }
                    kblexer_buf_init(lexer);
                    kblexer_special_init(lexer);
                }
                else {
                    kblexer_push_char(lexer, ch);
                }
            }
            else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
                kblexer_push_char(lexer, '\0');
                if (lexer->special_match.matched == COMMENT) {
                    lexer->state = LEXER_COMMENT;
                }
                else {
                    lexer->state = LEXER_NEWTOK;
                    kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(lexer->special_match.matched, NULL, lexer->tokline, lexer->tokcol));
                }
                kblexer_buf_init(lexer);
                kblexer_special_init(lexer);
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else {
                fprintf(stderr, "Unexpected token %c at %d:%d\n", ch, lexer->line, lexer->col);
                exit(1);
            }
            break;
        case LEXER_NEWTOK:
            lexer->tokline = lexer->line;
            lexer->tokcol = lexer->col;
            if (ch == '"') {
                lexer->state = LEXER_STRINGLIT;
            }
            else if (is_delim(ch)) {
                if(ch == '\n') {
                    lexer->state = LEXER_NEWLINE;
                    kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                }
            }
            else if (is_sep(ch)) { 
                lexer->state = LEXER_SYM; 
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else if (ch >= '0' && ch <= '9') {
                lexer->state = LEXER_NUMLIT;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
                lexer->state = LEXER_IDENTIFIER;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else {
                fprintf(stderr, "Unexpected token %c at %d:%d\n", ch, lexer->line, lexer->col);
                exit(1);
            }
            break;
        case LEXER_NUMLIT:
            if (ch >= '0' || ch <= '9' || ch == '.') {
                kblexer_push_char(lexer, ch);
                kblexer_int_next(lexer, ch);
                kblexer_float_next(lexer, ch);
                if (!lexer->int_match.is_integer && !lexer->float_match.is_float) {
                    fprintf(stderr, "Unexpected token %c at %d:%d\n", ch, lexer->line, lexer->col);
                    exit(1);
                }
            }
            else if(is_delim(ch) || is_sep(ch)) {
                kblexer_push_char(lexer, '\0');
                kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make((lexer->int_match.is_integer)?INT:FLOAT, lexer->buffer, lexer->tokline, lexer->tokcol));
                
                kblexer_int_init(lexer);
                kblexer_float_init(lexer);
                kblexer_buf_init(lexer);

                if (is_sep(ch)) {
                    lexer->state = LEXER_SYM;
                    kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                }
                else if(ch == '\n') {
                    lexer->state = LEXER_NEWLINE;
                    kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                }
                else {
                    lexer->state = LEXER_NEWTOK;
                }
            }
            else {
                fprintf(stderr, "Unexpected token %c at %d:%d\n", ch, lexer->line, lexer->col);
                exit(1);
            }
            break;
        case LEXER_IDENTIFIER:
            if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_') {
                kblexer_push_char(lexer, ch);
            }
            else if(is_delim(ch) || is_sep(ch)) {
                kblexer_push_char(lexer, '\0');
                kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(IDENTIFIER, lexer->buffer, lexer->tokline, lexer->tokcol));
                kblexer_buf_init(lexer);

                if (is_sep(ch)) {
                    lexer->state = LEXER_SYM;
                    kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                }
                else if(ch == '\n') {
                    lexer->state = LEXER_NEWLINE;
                    kblexer_next(lexer, tokens, num_tokens, capacity, ch);
                }
                else {
                    lexer->state = LEXER_NEWTOK;
                }
            }
            else {
                fprintf(stderr, "Unexpected token %c at %d:%d\n", ch, lexer->line, lexer->col);
                exit(1);
            }
            break;
        case LEXER_STRINGLIT:
            if (ch == '"') {
                lexer->buffer[lexer->cursor] = '\0';
                kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(STR, lexer->buffer, lexer->tokline, lexer->tokcol));
                lexer->cursor = 0;
                lexer->state = LEXER_NEWTOK;
            }
            else {
                if (lexer->cursor == lexer->buffer_size) {
                    lexer->buffer_size = 2 * lexer->buffer_size;
                    lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                }
                lexer->buffer[lexer->cursor] = ch;
                lexer->cursor++;
            }
            break;
        case LEXER_COMMENT:
            if (ch == '\n') {
                kblexer_push_char(lexer, '\0');
                kblexer_push_token(tokens, num_tokens, capacity, kbtoken_make(COMMENT, lexer->buffer, lexer->tokline, lexer->tokcol));
                kblexer_buf_init(lexer);
                lexer->state = LEXER_NEWLINE;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else {
                kblexer_push_char(lexer, ch);
            }
            break;
    }
}

struct kbtoken * kblexer_start(struct kblexer * lexer, struct kbsrc * src, unsigned int * num_tokens) { 
    *num_tokens = 0;
    unsigned int capacity = 2;
    struct kbtoken * tokens = malloc(sizeof(struct kbtoken) * capacity);
    for (int i = 0; i < src->length; i++) {
        if (src->content[i] == '\n') {
            lexer->line ++;
            lexer->col = 0;
        }
        else {
            lexer->col ++;
        }
        kblexer_next(lexer, &tokens, num_tokens, &capacity, src->content[i]);
    }

    while (lexer->indent_level-- > 0) {
        tokens[*num_tokens] = kbtoken_make(DEDENT, NULL, lexer->line, lexer->col);
        (*num_tokens)++;
    }

    kblexer_next(lexer, &tokens, num_tokens, &capacity, '\n');
    kblexer_push_token(&tokens, num_tokens, &capacity, kbtoken_make(END, NULL, lexer->line, lexer->col));
    
    return tokens;
}
