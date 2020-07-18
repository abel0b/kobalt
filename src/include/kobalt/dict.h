#ifndef KBDICT__H
#define KBDICT__H

struct kbdictentry {
    char * key;
    void * value;
};

struct kbdict {
    int numbuckets;
    struct kbdictentry** buckets;
    int * sizes;
    int * capacities;
};

void kbdict_new(struct kbdict * dict);
void kbdict_set(struct kbdict * dict, char * key, void * value);
void * kbdict_get(struct kbdict * dict, char * key);
void kbdict_display(struct kbdict * dict);
void kbdict_del(struct kbdict * dict);

#endif
