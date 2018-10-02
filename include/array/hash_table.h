#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define HASH_TABLE_SIZE 41

struct HashTable {
    int size;
    struct HashTableEntry * * table;
};

struct HashTableEntry {
    char * key;
    void * value;
    struct HashTableEntry * next;
};

int hash(char * key);

struct HashTable * hash_table_make();

void hash_table_set(struct HashTable * hash_table, char * key, void * value);

void * hash_table_get(struct HashTable * hash_table, char * key);

void hash_table_destroy(struct HashTable * hash_table);

#endif
