#include "klbase/path.h"
#include "klbase/fs.h"
#include <assert.h>
#include <stdarg.h>

void kl_path_push(struct kl_str* path, char* subpath) {
    assert(path->len);
    if (isds(path->data[path->len - 1])) {
        kl_str_cat(path, subpath);
    }
    else {
        kl_str_catf(path, "/%s", subpath);
    }
}

void kl_path_normalize(struct kl_str* path) {
    // TODO: get rid of ../ ./

    int i = 0;
    if (path->len && path->data[0] == '.') {
        i = 1;
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

char* kl_path_basename(struct kl_str* path) {
    
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
