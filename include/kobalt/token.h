#ifndef KOBALT__TOKEN__H
#define KOBALT__TOKEN__H

struct Token {
    char * value;
    short int size;
};

struct Token * token_make(char * str);

void token_delete();

#endif
