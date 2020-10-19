#ifndef KBLOG__H
#define KBLOG__H

#include <stdio.h>
#include "kobalt/kobalt.h"

// TODO: multithreading support

// Regular text
#define BLK "\x1B[0;30m"
#define RED "\x1B[0;31m"
#define GRN "\x1B[0;32m"
#define YEL "\x1B[0;33m"
#define BLU "\x1B[0;34m"
#define MAG "\x1B[0;35m"
#define CYN "\x1B[0;36m"
#define WHT "\x1B[0;37m"

// Regular bold text
#define BBLK "\x1B[1;30m"
#define BRED "\x1B[1;31m"
#define BGRN "\x1B[1;32m"
#define BYEL "\x1B[1;33m"
#define BBLU "\x1B[1;34m"
#define BMAG "\x1B[1;35m"
#define BCYN "\x1B[1;36m"
#define BWHT "\x1B[1;37m"

// Regular underline text
#define UBLK "\x1B[4;30m"
#define URED "\x1B[4;31m"
#define UGRN "\x1B[4;32m"
#define UYEL "\x1B[4;33m"
#define UBLU "\x1B[4;34m"
#define UMAG "\x1B[4;35m"
#define UCYN "\x1B[4;36m"
#define UWHT "\x1B[4;37m"

// Regular background
#define BLKB "\x1B[40m"
#define REDB "\x1B[41m"
#define GRNB "\x1B[42m"
#define YELB "\x1B[43m"
#define BLUB "\x1B[44m"
#define MAGB "\x1B[45m"
#define CYNB "\x1B[46m"
#define WHTB "\x1B[47m"

// Reset
#define RESET "\x1B[0m"

#if DEBUG
#define kbelog(...)\
    do {\
        fprintf(stderr, BRED "error: " RESET " (" __FILE__ ":%d" ") ", __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, "\n");\
        fflush(stderr);\
    } while(0)
#else
#define kbelog(...)\
    do {\
        fprintf(stderr, BRED "error:" RESET " ");\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, "\n");\
        fflush(stderr);\
    } while(0)
#endif

#define kbwlog(...)\
    do {\
        fprintf(stderr, BYEL "warning:" RESET " ");\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, "\n");\
        fflush(stderr);\
    } while(0)

#define kbilog(...)\
    do {\
        fprintf(stdout, BBLU "info:" RESET " ");\
        fprintf(stdout, __VA_ARGS__);\
        fprintf(stdout, "\n");\
        fflush(stdout);\
    } while(0)

#endif
