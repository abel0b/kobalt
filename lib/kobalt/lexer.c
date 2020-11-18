#include "kobalt/lexer.h"
#include "kobalt/source.h"
#include "kobalt/memory.h"
#include "kobalt/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

void kblex(struct kbsrc* src, struct kbvec_token* tokens) {
    struct kblexer lexer;
    kblexer_new(&lexer);
    kbvec_token_new(tokens);
    kblexer_run(&lexer, src, tokens);
    kblexer_del(&lexer);
}
        
void kblexer_new(struct kblexer* lexer) {
    lexer->state = LEXER_NEWLINE;
    lexer->incomment = 0;
    lexer->indent_level = 0;
    lexer->newline = 1;
    lexer->indent_tab = 1;
    lexer->first_indent = 1;
    lexer->first_indent_char = 1;
    lexer->use_tabs_indent = 0;
    lexer->indent_counter = 0;
    lexer->space_indent = -1;
    lexer->prev_col = 0;
    lexer->line = 1;
    lexer->col = 0;
    kblexer_special_init(lexer);
    kblexer_int_init(lexer);
    kblexer_float_init(lexer);
    lexer->buffer_size = 16;
    lexer->buffer = kbmalloc(lexer->buffer_size * sizeof (char));
    lexer->cursor = 0;
}

void kblexer_del(struct kblexer * lexer) {
    kbfree(lexer->buffer);
}

void kblexer_special_init(struct kblexer * lexer) {
    lexer->special_match.matched = TILLEGAL;
    lexer->special_match.nummatched = TILLEGAL+1;
    lexer->special_match.cursor = 0;
    int i;
    for (i=0; i<=TILLEGAL; ++i) {
        lexer->special_match.match[i] = 1;
    }
}

void kblexer_special_next(struct kblexer * lexer, char ch) {
    lexer->special_match.matched = TILLEGAL;
    int i;
    for (i=0; i<=TILLEGAL; ++i) {
        if (lexer->special_match.match[i]) {
            int speclen = strlen(specials[i]);
            if (speclen <= lexer->special_match.cursor || specials[i][lexer->special_match.cursor] != ch) {
                lexer->special_match.match[i] = 0;
                lexer->special_match.nummatched --;
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

void kblexer_push_token(struct kblexer* lexer, struct kbvec_token* tokens, struct kbtoken token) {
    kbvec_token_push(tokens, token);
    if (lexer->incomment && token.kind != TComment) {
        lexer->incomment = 0;
    }
#if DEBUG
    if(getenv("DEBUG_LEXER")) printf("push  %s=%s\n", kbtoken_string(token.kind), (token.value)? token.value : "");
#endif
}

void kblexer_push_char(struct kblexer * lexer, char ch) {
    if (lexer->cursor == lexer->buffer_size) {
        lexer->buffer_size = 2 * lexer->buffer_size;
        lexer->buffer = kbrealloc(lexer->buffer, lexer->buffer_size * sizeof(char));
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
        case LEXER_CHARLIT:
            return "CHARLIT";
        case LEXER_NUMLIT:
            return "NUMLIT";
        case LEXER_COMMENT:
            return "COMMENT";
        case LEXER_IDENTIFIER:
            return "IDENTIFIER";
    }
    return "UNDEFINED";
}

static void unexpectedchar(char ch, int line, int col) {
    if (isprint(ch)) {
        kbelog("unexpected character '%c' at %d:%d", ch, line, col);
    }
    else {
        kbelog("unexpected character '\\x%x' at %d:%d", ch, line, col);
    }
}

void kblexer_next(struct kblexer* lexer, struct kbvec_token* tokens, char ch) {
#if DEBUG
    if(getenv("DEBUG_LEXER")) printf("lex cursor=%c state=%s\n", ch, kblexer_state_str(lexer));
#endif
    switch(lexer->state) {
        case LEXER_NEWLINE:
            {
                if(tokens->size && kbvec_token_last(tokens).kind != TLineFeed && !lexer->incomment) {
                    kblexer_push_token(lexer, tokens, kbtoken_make(TLineFeed, NULL, lexer->line - 1, lexer->prev_col));
                }

                if (ch == ' ') {
                    if (!lexer->first_indent_char && lexer->use_tabs_indent) {
                        kbelog("inconsistent use of tabs and space in indentation");
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
                        kbelog("inconsistent use of tabs and space in indentation");
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
                        }
                    }
                    else if(lexer->indent_counter % lexer->space_indent != 0) {
                        kbelog("unexpected indentation at %d:%d", lexer->line, lexer->col);
                        exit(1);
                    }

                    if ((lexer->indent_counter % lexer->space_indent == 0) && (lexer->indent_counter/lexer->space_indent > lexer->indent_level)) {
                        for(int kk=0; kk<(lexer->indent_counter / lexer->space_indent - lexer->indent_level); ++kk)
                            kblexer_push_token(lexer, tokens, kbtoken_make(TIndent, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }

                    if ((lexer->indent_counter % lexer->space_indent == 0) && (lexer->indent_counter/lexer->space_indent < lexer->indent_level)) {
                        for(int kk=0; kk<(lexer->indent_level - lexer->indent_counter / lexer->space_indent); ++kk)
                            kblexer_push_token(lexer, tokens, kbtoken_make(TDedent, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }

                    lexer->indent_counter = 0;
                    lexer->state = LEXER_NEWTOK;
                    kblexer_next(lexer, tokens, ch);
                }
            }
            break;
        case LEXER_SYM:
            {
                if (is_delim(ch)) {
                    kblexer_push_char(lexer, '\0');
                    
                    if (lexer->special_match.matched == TSemi) {
                        lexer->state = LEXER_COMMENT;
                    }
                    else {
                        if (is_builtin_fun(lexer->special_match.matched)) {
                            kblexer_push_token(lexer, tokens, kbtoken_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                        }
                        else {
                            kblexer_push_token(lexer, tokens, kbtoken_make(lexer->special_match.matched, (lexer->special_match.matched == TILLEGAL)? lexer->buffer : NULL, lexer->tokline, lexer->tokcol));
                        }
                    }

                    kblexer_buf_init(lexer);
                    kblexer_special_init(lexer);

                    if (ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kblexer_next(lexer, tokens, ch);
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                    }
                }
                else if (is_sep(ch)) {
                    int prevmatched = lexer->special_match.matched;
                    kblexer_special_next(lexer, ch); 
                    if (lexer->special_match.nummatched == 0) {
                        if (lexer->cursor > 0) {
                            if (prevmatched == TSemi) {
                                lexer->state = LEXER_COMMENT;
                            }
                            else {
                                kblexer_push_char(lexer, ch);
                                kblexer_push_char(lexer, '\0');
                                lexer->state = LEXER_NEWTOK;
                                if (is_builtin_fun(lexer->special_match.matched)) {
                                    kblexer_push_token(lexer, tokens, kbtoken_make(TId, lexer->buffer, lexer->line, lexer->col));
                                }
                                else {
                                    kblexer_push_token(lexer, tokens, kbtoken_make(prevmatched, (prevmatched == TILLEGAL)? lexer->buffer : NULL, lexer->line, lexer->col));
                                }
                            }       
                            
                            kblexer_special_init(lexer);
                            kblexer_buf_init(lexer);
                            kblexer_next(lexer, tokens, ch);
                        }
                        else {
                            unexpectedchar(ch, lexer->line, lexer->col);
                            exit(1);
                        }
                    }
                    else if (lexer->special_match.nummatched == 1 && lexer->special_match.matched != TILLEGAL) {
                        if (lexer->special_match.matched == TSemi) {
                            lexer->state = LEXER_COMMENT;
                        }
                        else {
                            kblexer_push_char(lexer, ch);
                            kblexer_push_char(lexer, '\0');
                            lexer->state = LEXER_NEWTOK;
                            if (is_builtin_fun(lexer->special_match.matched)) {
                                kblexer_push_token(lexer, tokens, kbtoken_make(TId, lexer->buffer, lexer->line, lexer->col));
                            }
                            else {
                                kblexer_push_token(lexer, tokens, kbtoken_make(lexer->special_match.matched, NULL, lexer->line, lexer->col));
                            }
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
                    if (lexer->special_match.matched == TSemi) {
                        lexer->state = LEXER_COMMENT;
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                        if (is_builtin_fun(lexer->special_match.matched)) {
                            kblexer_push_token(lexer, tokens, kbtoken_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                        }
                        else {
                            kblexer_push_token(lexer, tokens, kbtoken_make(lexer->special_match.matched, NULL, lexer->tokline, lexer->tokcol));
                        }
                    }
                    kblexer_buf_init(lexer);
                    kblexer_special_init(lexer);
                    kblexer_next(lexer, tokens, ch);
                }
                else {
                    unexpectedchar(ch, lexer->line, lexer->col);
                    exit(1);
                }
            }
            break;
        case LEXER_NEWTOK:
            {
                lexer->tokline = lexer->line;
                lexer->tokcol = lexer->col;
                if (ch == '"') {
                    lexer->state = LEXER_STRINGLIT;
                }
                else if (ch == '\'') {
                    lexer->state = LEXER_CHARLIT;
                }
                else if (is_delim(ch)) {
                    if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kblexer_next(lexer, tokens, ch);
                    }
                }
                else if (is_sep(ch)) { 
                    lexer->state = LEXER_SYM; 
                    kblexer_next(lexer, tokens, ch);
                }
                else if (ch >= '0' && ch <= '9') {
                    lexer->state = LEXER_NUMLIT;
                    kblexer_next(lexer, tokens, ch);
                }
                else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
                    lexer->state = LEXER_IDENTIFIER;
                    kblexer_next(lexer, tokens, ch);
                }
                else {
                    unexpectedchar(ch, lexer->line, lexer->col);
                    exit(1);
                }
            }
            break;
        case LEXER_NUMLIT:
            {
                if ((ch >= '0' && ch <= '9') || ch == '.') {
                    kblexer_push_char(lexer, ch);
                    kblexer_int_next(lexer, ch);
                    kblexer_float_next(lexer, ch);
                    if (!lexer->int_match.is_integer && !lexer->float_match.is_float) {
                        unexpectedchar(ch, lexer->line, lexer->col);
                        exit(1);
                    }
                }
                else if(is_delim(ch) || is_sep(ch)) {
                    kblexer_push_char(lexer, '\0');
                    kblexer_push_token(lexer, tokens, kbtoken_make((lexer->int_match.is_integer)?TInt:TFloat, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kblexer_buf_init(lexer);

                    kblexer_int_init(lexer);
                    kblexer_float_init(lexer);

                    if (is_sep(ch)) {
                        lexer->state = LEXER_SYM;
                        kblexer_next(lexer, tokens, ch);
                    }
                    else if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kblexer_next(lexer, tokens, ch);
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                    }
                }
                else {
                    unexpectedchar(ch, lexer->line, lexer->col);
                    exit(1);
                }
            }
            break;
        case LEXER_IDENTIFIER:
            {
                if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_' || ch == ':') {
                    kblexer_push_char(lexer, ch);
                }
                else if(is_delim(ch) || is_sep(ch)) {
                    kblexer_push_char(lexer, '\0');
                    kblexer_push_token(lexer, tokens, kbtoken_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kblexer_buf_init(lexer);

                    if (is_sep(ch)) {
                        lexer->state = LEXER_SYM;
                        kblexer_next(lexer, tokens, ch);
                    }
                    else if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kblexer_next(lexer, tokens, ch);
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                    }
                }
                else {
                    unexpectedchar(ch, lexer->line, lexer->col);
                    exit(1);
                }
            }
            break;
        case LEXER_STRINGLIT:
            {
                if (ch == '"') {
                    kblexer_push_char(lexer, '\0');
                    kblexer_push_token(lexer, tokens, kbtoken_make(TStr, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kblexer_buf_init(lexer);
                    lexer->state = LEXER_NEWTOK;
                }
                else {
                    // TODO: check valid characters
                    kblexer_push_char(lexer, ch);
                }
            }
            break;
        case LEXER_CHARLIT:
            {
                if (ch == '\'') {
                    kblexer_push_char(lexer, '\0');
                    kblexer_push_token(lexer, tokens, kbtoken_make(TChar, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kblexer_buf_init(lexer);
                    lexer->state = LEXER_NEWTOK;
                }
                else {
                    // TODO: check valid characters
                    if (lexer->cursor && lexer->buffer[0] != '\\') {
                        kbelog("multi-character constant");
                        exit(1);
                    }
                    kblexer_push_char(lexer, ch);
                }
            }
            break;
        case LEXER_COMMENT:
            {
                lexer->incomment = 1;
                if (ch == '\n') {
                    kblexer_push_char(lexer, '\0');
                    kblexer_push_token(lexer, tokens, kbtoken_make(TComment, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kblexer_buf_init(lexer);
                    lexer->state = LEXER_NEWLINE;
                    kblexer_next(lexer, tokens, ch);
                }
                else {
                    kblexer_push_char(lexer, ch);
                }
            }
            break;
    }
}

void kblexer_run(struct kblexer* lexer, struct kbsrc* src, struct kbvec_token* tokens) { 
    for (int i = 0; i < src->length; ++i) {
        if (src->content[i] == '\n') {
            lexer->prev_col = lexer->col;
            lexer->line ++;
            lexer->col = 0;
        }
        else {
            lexer->col ++;
        }
        kblexer_next(lexer, tokens, src->content[i]);
    }
    
    // add line feed if not present
    if (src->content[src->length - 1] != '\n') kblexer_next(lexer, tokens, '\n');

    while (lexer->indent_level-- > 0) {
        kblexer_push_token(lexer, tokens, kbtoken_make(TDedent, NULL, lexer->line, lexer->col));
    }

    kblexer_push_token(lexer, tokens, kbtoken_make(TEndFile, NULL, lexer->line, lexer->col));
}
