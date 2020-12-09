#include "kobalt/lexer.h"
#include "klbase/mem.h"
#include "klbase/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

enum kl_lexer_state {
    LEXER_NEWLINE,
    LEXER_NEWTOK,
    LEXER_SYM,
    LEXER_STRINGLIT,
    LEXER_CHARLIT,
    LEXER_NUMLIT,
    LEXER_COMMENT,
    LEXER_IDENTIFIER,
};

struct kl_lexer {
    enum kl_lexer_state state;
    int incomment;
    int indent_level;
    int newline;
    int indent_tab;
    int first_indent;
    int use_tabs_indent;
    int first_indent_char;
    int indent_counter;
    int space_indent;
    int line;
    int prev_col;
    int col;
    int tokline;
    int tokcol;
    struct  {
        int cursor;
        int match[NUM_SPECIALS];
        int nummatched;
        int matched;
    } special_match;
    struct {
        int first;
        int is_integer;
    } int_match;
    struct {
        int is_float;
        int has_dot;
        int first;
    } float_match;
    int buffer_size;
    int cursor;
    char * buffer;
};
        

static void kl_lexer_special_init(struct kl_lexer * lexer) {
    lexer->special_match.matched = TILLEGAL;
    lexer->special_match.nummatched = TILLEGAL+1;
    lexer->special_match.cursor = 0;
    int i;
    for (i=0; i<=TILLEGAL; ++i) {
        lexer->special_match.match[i] = 1;
    }
}

static void kl_lexer_float_init(struct kl_lexer * lexer) {
    lexer->float_match.is_float = 1;
    lexer->float_match.has_dot = 0;
    lexer->float_match.first = 1;
}

static void kl_lexer_int_init(struct kl_lexer * lexer) {
    lexer->int_match.is_integer = 1;
    lexer->int_match.first = 1;
}

static void kl_lexer_new(struct kl_lexer* lexer) {
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
    kl_lexer_special_init(lexer);
    kl_lexer_int_init(lexer);
    kl_lexer_float_init(lexer);
    lexer->buffer_size = 16;
    lexer->buffer = kl_malloc(lexer->buffer_size * sizeof (char));
    lexer->cursor = 0;
}

static void kl_lexer_del(struct kl_lexer * lexer) {
    kl_free(lexer->buffer);
}

static void kl_lexer_special_next(struct kl_lexer * lexer, char ch) {
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

static void kl_lexer_float_next(struct kl_lexer * lexer, char ch) {
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



static void kl_lexer_int_next(struct kl_lexer * lexer, char ch) {
    if (lexer->int_match.first) {
        lexer->int_match.first = 0;
        if (ch == '-') return;
    }
    if(!(ch >= '0' && ch <= '9')) {
        lexer->int_match.is_integer = 0;
    }
}

static void kl_lexer_push_token(struct kl_lexer* lexer, struct kl_vec_token* tokens, struct kl_token token) {
    kl_vec_token_push(tokens, token);
    if (lexer->incomment && token.kind != TComment) {
        lexer->incomment = 0;
    }
#if DEBUG
    if(getenv("DEBUG_LEXER")) printf("push  %s=%s\n", kl_token_string(token.kind), (token.value)? token.value : "");
#endif
}

static void kl_lexer_push_char(struct kl_lexer * lexer, char ch) {
    if (lexer->cursor == lexer->buffer_size) {
        lexer->buffer_size = 2 * lexer->buffer_size;
        lexer->buffer = kl_realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
    }
    lexer->buffer[lexer->cursor] = ch;
    lexer->cursor++;
}

static void kl_lexer_buf_init(struct kl_lexer * lexer) {
    lexer->cursor = 0;
}

#if DEBUG
static char * kl_lexer_state_str(struct kl_lexer * lexer) {
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
#endif

static void unexpectedchar(char ch, int line, int col) {
    if (isprint(ch)) {
        kl_elog("unexpected character '%c' at %d:%d", ch, line, col);
    }
    else {
        kl_elog("unexpected character '\\x%x' at %d:%d", ch, line, col);
    }
}

static void kl_lexer_next(struct kl_lexer* lexer, struct kl_vec_token* tokens, char ch) {
#if DEBUG
    if(getenv("DEBUG_LEXER")) {
        printf("lex cursor=%c state=%s\n", ch, kl_lexer_state_str(lexer));
    }
#endif
    switch(lexer->state) {
        case LEXER_NEWLINE:
            {
                if(tokens->size && kl_vec_token_last(tokens).kind != TLineFeed && !lexer->incomment) {
                    kl_lexer_push_token(lexer, tokens, kl_token_make(TLineFeed, NULL, lexer->line - 1, lexer->prev_col));
                }

                if (ch == ' ') {
                    if (!lexer->first_indent_char && lexer->use_tabs_indent) {
                        kl_elog("inconsistent use of tabs and space in indentation");
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
                        kl_elog("inconsistent use of tabs and space in indentation");
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
                        kl_elog("unexpected indentation at %d:%d", lexer->line, lexer->col);
                        exit(1);
                    }

                    if ((lexer->indent_counter % lexer->space_indent == 0) && (lexer->indent_counter/lexer->space_indent > lexer->indent_level)) {
                        for(int kk=0; kk<(lexer->indent_counter / lexer->space_indent - lexer->indent_level); ++kk)
                            kl_lexer_push_token(lexer, tokens, kl_token_make(TIndent, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }

                    if ((lexer->indent_counter % lexer->space_indent == 0) && (lexer->indent_counter/lexer->space_indent < lexer->indent_level)) {
                        for(int kk=0; kk<(lexer->indent_level - lexer->indent_counter / lexer->space_indent); ++kk)
                            kl_lexer_push_token(lexer, tokens, kl_token_make(TDedent, NULL, lexer->line, 1));
                        lexer->indent_level = lexer->indent_counter / lexer->space_indent;
                    }

                    lexer->indent_counter = 0;
                    lexer->state = LEXER_NEWTOK;
                    kl_lexer_next(lexer, tokens, ch);
                }
            }
            break;
        case LEXER_SYM:
            {
                if (is_delim(ch)) {
                    kl_lexer_push_char(lexer, '\0');
                    
                    if (lexer->special_match.matched == TSemi) {
                        lexer->state = LEXER_COMMENT;
                    }
                    else {
                        if (is_builtin_fun(lexer->special_match.matched)) {
                            kl_lexer_push_token(lexer, tokens, kl_token_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                        }
                        else {
                            kl_lexer_push_token(lexer, tokens, kl_token_make(lexer->special_match.matched, (lexer->special_match.matched == TILLEGAL)? lexer->buffer : NULL, lexer->tokline, lexer->tokcol));
                        }
                    }

                    kl_lexer_buf_init(lexer);
                    kl_lexer_special_init(lexer);

                    if (ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kl_lexer_next(lexer, tokens, ch);
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                    }
                }
                else if (is_sep(ch)) {
                    int prevmatched = lexer->special_match.matched;
                    kl_lexer_special_next(lexer, ch); 
                    if (lexer->special_match.nummatched == 0) {
                        if (lexer->cursor > 0) {
                            if (prevmatched == TSemi) {
                                lexer->state = LEXER_COMMENT;
                            }
                            else {
                                kl_lexer_push_char(lexer, ch);
                                kl_lexer_push_char(lexer, '\0');
                                lexer->state = LEXER_NEWTOK;
                                if (is_builtin_fun(lexer->special_match.matched)) {
                                    kl_lexer_push_token(lexer, tokens, kl_token_make(TId, lexer->buffer, lexer->line, lexer->col));
                                }
                                else {
                                    kl_lexer_push_token(lexer, tokens, kl_token_make(prevmatched, (prevmatched == TILLEGAL)? lexer->buffer : NULL, lexer->line, lexer->col));
                                }
                            }       
                            
                            kl_lexer_special_init(lexer);
                            kl_lexer_buf_init(lexer);
                            kl_lexer_next(lexer, tokens, ch);
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
                            kl_lexer_push_char(lexer, ch);
                            kl_lexer_push_char(lexer, '\0');
                            lexer->state = LEXER_NEWTOK;
                            if (is_builtin_fun(lexer->special_match.matched)) {
                                kl_lexer_push_token(lexer, tokens, kl_token_make(TId, lexer->buffer, lexer->line, lexer->col));
                            }
                            else {
                                kl_lexer_push_token(lexer, tokens, kl_token_make(lexer->special_match.matched, NULL, lexer->line, lexer->col));
                            }
                        }
                        kl_lexer_buf_init(lexer);
                        kl_lexer_special_init(lexer);
                    }
                    else {
                        kl_lexer_push_char(lexer, ch);
                    }
                }
                else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
                    kl_lexer_push_char(lexer, '\0');
                    if (lexer->special_match.matched == TSemi) {
                        lexer->state = LEXER_COMMENT;
                    }
                    else {
                        lexer->state = LEXER_NEWTOK;
                        if (is_builtin_fun(lexer->special_match.matched)) {
                            kl_lexer_push_token(lexer, tokens, kl_token_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                        }
                        else {
                            kl_lexer_push_token(lexer, tokens, kl_token_make(lexer->special_match.matched, NULL, lexer->tokline, lexer->tokcol));
                        }
                    }
                    kl_lexer_buf_init(lexer);
                    kl_lexer_special_init(lexer);
                    kl_lexer_next(lexer, tokens, ch);
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
                        kl_lexer_next(lexer, tokens, ch);
                    }
                }
                else if (is_sep(ch)) { 
                    lexer->state = LEXER_SYM; 
                    kl_lexer_next(lexer, tokens, ch);
                }
                else if (ch >= '0' && ch <= '9') {
                    lexer->state = LEXER_NUMLIT;
                    kl_lexer_next(lexer, tokens, ch);
                }
                else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
                    lexer->state = LEXER_IDENTIFIER;
                    kl_lexer_next(lexer, tokens, ch);
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
                    kl_lexer_push_char(lexer, ch);
                    kl_lexer_int_next(lexer, ch);
                    kl_lexer_float_next(lexer, ch);
                    if (!lexer->int_match.is_integer && !lexer->float_match.is_float) {
                        unexpectedchar(ch, lexer->line, lexer->col);
                        exit(1);
                    }
                }
                else if(is_delim(ch) || is_sep(ch)) {
                    kl_lexer_push_char(lexer, '\0');
                    kl_lexer_push_token(lexer, tokens, kl_token_make((lexer->int_match.is_integer)?TInt:TFloat, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kl_lexer_buf_init(lexer);

                    kl_lexer_int_init(lexer);
                    kl_lexer_float_init(lexer);

                    if (is_sep(ch)) {
                        lexer->state = LEXER_SYM;
                        kl_lexer_next(lexer, tokens, ch);
                    }
                    else if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kl_lexer_next(lexer, tokens, ch);
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
                    kl_lexer_push_char(lexer, ch);
                }
                else if(is_delim(ch) || is_sep(ch)) {
                    kl_lexer_push_char(lexer, '\0');
                    kl_lexer_push_token(lexer, tokens, kl_token_make(TId, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kl_lexer_buf_init(lexer);

                    if (is_sep(ch)) {
                        lexer->state = LEXER_SYM;
                        kl_lexer_next(lexer, tokens, ch);
                    }
                    else if(ch == '\n') {
                        lexer->state = LEXER_NEWLINE;
                        kl_lexer_next(lexer, tokens, ch);
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
                    kl_lexer_push_char(lexer, '\0');
                    kl_lexer_push_token(lexer, tokens, kl_token_make(TStr, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kl_lexer_buf_init(lexer);
                    lexer->state = LEXER_NEWTOK;
                }
                else {
                    // TODO: check valid characters
                    kl_lexer_push_char(lexer, ch);
                }
            }
            break;
        case LEXER_CHARLIT:
            {
                if (ch == '\'') {
                    kl_lexer_push_char(lexer, '\0');
                    kl_lexer_push_token(lexer, tokens, kl_token_make(TChar, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kl_lexer_buf_init(lexer);
                    lexer->state = LEXER_NEWTOK;
                }
                else {
                    // TODO: check valid characters
                    if (lexer->cursor && lexer->buffer[0] != '\\') {
                        kl_elog("multi-character constant");
                        exit(1);
                    }
                    kl_lexer_push_char(lexer, ch);
                }
            }
            break;
        case LEXER_COMMENT:
            {
                lexer->incomment = 1;
                if (ch == '\n') {
                    kl_lexer_push_char(lexer, '\0');
                    kl_lexer_push_token(lexer, tokens, kl_token_make(TComment, lexer->buffer, lexer->tokline, lexer->tokcol));
                    kl_lexer_buf_init(lexer);
                    lexer->state = LEXER_NEWLINE;
                    kl_lexer_next(lexer, tokens, ch);
                }
                else {
                    kl_lexer_push_char(lexer, ch);
                }
            }
            break;
    }
}

static void kl_lexer_run(struct kl_lexer* lexer, struct kl_compiland* compiland, struct kl_vec_token* tokens) {
    for (int i = 0; i < compiland->content.len; ++ i) {
        if (compiland->content.data[i] == '\r' && i < compiland->content.len - 1 && compiland->content.data[i + 1] == '\n') {
            continue;
        }
        if (compiland->content.data[i] == '\n') {
            lexer->prev_col = lexer->col;
            ++ lexer->line;
            lexer->col = 0;
        }
        else {
            ++ lexer->col;
        }
        kl_lexer_next(lexer, tokens, compiland->content.data[i]);
    }
    
    // add line feed if not present
    if (compiland->content.len && compiland->content.data[compiland->content.len - 1] != '\n') {
        kl_lexer_next(lexer, tokens, '\n');
    }

    while (lexer->indent_level-- > 0) {
        kl_lexer_push_token(lexer, tokens, kl_token_make(TDedent, NULL, lexer->line, lexer->col));
    }

    kl_lexer_push_token(lexer, tokens, kl_token_make(TEndFile, NULL, lexer->line, lexer->col));
}

void kl_lex(struct kl_compiland* compiland, struct kl_vec_token* tokens) {
    struct kl_lexer lexer;
    kl_lexer_new(&lexer);
    kl_vec_token_new(tokens);
    kl_lexer_run(&lexer, compiland, tokens);
    kl_lexer_del(&lexer);
}
