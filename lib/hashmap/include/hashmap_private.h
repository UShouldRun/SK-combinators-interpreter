#ifndef HASH_MAP_PRIVATE_H
#define HASH_MAP_PRIVATE_H

#include "hashmap.h"

// ==================================================# PRIVATE #=================================================================

typedef struct hashmap_node {
  char *key;
  void *value;
  struct hashmap_node *next;
} *HashMapNode;

struct hashmap {
  uint64_t s_buckets, s_elements;
  float    load_threshold_factor;
  struct hashmap_node buckets[];
};

const float min_load_threshold_factor = .75;

bool     hashmap_near_capacity (HashMap hashmap);
uint64_t hashmap_next_power_2  (uint64_t n);
uint64_t hash                  (char *key, uint64_t s_buckets);

#endif // !HASHMAP_PRIVATE
