#include "kobalt/path.h"
#include "kobalt/fs.h"
#include <assert.h>
#include <stdarg.h>

void kbpath_push(struct kbstr* path, char* subpath) {
    assert(path->len);
    if (isds(path->data[path->len - 1])) {
        kbstr_cat(path, subpath);
    }
    else {
        kbstr_catf(path, "/%s", subpath);
    }
}

void kbpath_normalize(struct kbstr* path) {
    // TODO: get rid of ../ ./

    int i = 0;
    if (path->len && path->data[0] == '.') {
        while(isds(path->data[i])) {
            i ++;
        }
    }

    int cur = 0;
    while(i < path->len) {
        if(isds(path->data[i])) {
            path->data[cur++] = '/';
            while(isds(path->data[i])) {
                i++;
            }
        }
        else {
            path->data[cur++] = path->data[i++];
        }
    }

    if (isds(path->len && path->data[cur - 1])) {
        -- cur;
    }

    path->data[cur] = '\0';
    path->len = cur;
}

char* kbpath_basename(struct kbstr* path) {
    
    int base = 0;
    {
        int i = 0;
        char* pathiter = path->data;
        while(*pathiter != '\0') {
            if (isds(*pathiter)){
                base = i+1;
            }
            ++ i;
            ++ pathiter;
        }
    }
    return path->data + base;
}
