#include "array/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int hash(char * key) {
    int value = 0;
    for(int i = 0; i < strlen(key); i++) {
        value += (key[i] * HASH_TABLE_SIZE) % HASH_TABLE_SIZE;
    }
    return value % HASH_TABLE_SIZE;
}

struct HashTable * hash_table_make() {
    struct HashTable * hash_table = malloc (sizeof (struct HashTable));

    hash_table->size = HASH_TABLE_SIZE;
    hash_table->table = malloc (sizeof (struct HashTableEntry *) * hash_table->size);

    for (int i = 0; i < hash_table->size; i++) {
        hash_table->table[i] = NULL;
    }

    return hash_table;
}

void hash_table_set(struct HashTable * hash_table, char * key, void * value) {
    int hash_key = hash(key);

    struct HashTableEntry * entry = malloc (sizeof (struct HashTableEntry));
    entry->key = key;
    entry->value = value;

    if (hash_table->table[hash_key] == NULL) {
        entry->next = NULL;
    }
    else {
        entry->next = hash_table->table[hash_key];
    }
    hash_table->table[hash_key] = entry;
}

void * hash_table_get(struct HashTable * hash_table, char * key) {
    int hash_key = hash(key);
    struct HashTableEntry * entry = hash_table->table[hash_key];

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    if (entry == NULL)
        return NULL;

    return entry->value;
}

void hash_table_destroy(struct HashTable * hash_table) {
    struct HashTableEntry * entry;
    struct HashTableEntry * next;

    for (int i = 0; i < hash_table->size; i++) {
        for (entry = hash_table->table[i]; entry != NULL; entry = next) {
            next = entry->next;
            free (entry);
        }
    }
    free (hash_table);
}
