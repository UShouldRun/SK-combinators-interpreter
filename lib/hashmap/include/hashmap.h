#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// ==================================================# PUBLIC #=================================================================

typedef struct hashmap *HashMap;

HashMap  hashmap_create (uint64_t s_buckets, float load_threshold_factor);

uint64_t hashmap_size   (HashMap hashmap);

void*    hashmap_get    (HashMap hashmap, char* key);

bool     hashmap_exists (HashMap hashmap, char* key);
bool     hashmap_insert (HashMap* hashmap, char* key, void* value, void free_value(void* value), const bool);
bool     hashmap_remove (HashMap hashmap, char* key, void free_value(void* value), const bool);
bool     hashmap_free   (HashMap hashmap, void free_value(void* value), const bool);

#endif
