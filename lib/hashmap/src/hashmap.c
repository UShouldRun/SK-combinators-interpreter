#include "hashmap_private.h"

// ==================================================# PUBLIC #=================================================================

HashMap hashmap_create(uint64_t s_buckets, float load_threshold_factor) {
  if (s_buckets == 0)
    return NULL;
  if (load_threshold_factor < min_load_threshold_factor)
    return NULL;
  HashMap hashmap = (HashMap)calloc(1, sizeof(struct hashmap) + s_buckets * sizeof(struct hashmap_node));
  assert(hashmap != NULL);
  hashmap->s_buckets             = hashmap_next_power_2(s_buckets);
  hashmap->s_elements            = 0;
  hashmap->load_threshold_factor = load_threshold_factor;
  return hashmap;
}

uint64_t hashmap_size(HashMap hashmap) {
  if (hashmap == NULL)
    return 0;
  return hashmap->s_elements;
}

bool hashmap_exists(HashMap hashmap, char *key) {
  if (hashmap == NULL)
    return false;
  if (key == NULL)
    return false;
  uint64_t index = hash(key, hashmap->s_buckets);
  struct hashmap_node head = hashmap->buckets[index];
  if (head.key == NULL)
    return false;
  if (strcmp(head.key, key) == 0)
    return true;
  HashMapNode current = head.next;
  for (; current != NULL; current = current->next)
    if (strcmp(current->key, key) == 0)
      return true;
  return false;
}

bool hashmap_insert(HashMap *hashmap, char *key, void *value, void free_value(void* value)) {
  if (hashmap == NULL)
    return false;
  if (*hashmap == NULL)
    return false;
  if (key == NULL)
    return false;
  if (value == NULL)
    return false;

  uint64_t index = hash(key, (*hashmap)->s_buckets);
  HashMapNode head = &((*hashmap)->buckets[index]);

  if (head->key == NULL) {
    head->key = strdup(key);
    assert(head->key != NULL);
    head->value = value;
    head->next  = NULL;
    (*hashmap)->s_elements++;
    return true;
  } 

  if (strcmp(head->key, key) == 0) {
    if (free_value) {
      free_value(head->value);
    } else {
      free(head->value);
    }
    head->value = value;
    return true;
  }

  HashMapNode *current = &((*hashmap)->buckets[index].next);
  for (; *current != NULL; current = &((*current)->next)) {
    if (strcmp((*current)->key, key) != 0)
      continue;
    if (free_value) {
      free_value((*current)->value);
    } else {
      free((*current)->value);
    }
    (*current)->value = value;
    return true;
  }

  HashMapNode new_node = (HashMapNode)malloc(sizeof(struct hashmap_node));
  assert(new_node != NULL);
  new_node->key = strdup(key);
  assert(new_node->key != NULL);
  new_node->value = value;
  new_node->next = NULL;
  *current = new_node;
  (*hashmap)->s_elements++;

  if (!hashmap_near_capacity(*hashmap))
    return true;

  uint64_t new_s_buckets = 2 * (*hashmap)->s_buckets;
  assert(new_s_buckets > (*hashmap)->s_buckets);
  HashMap temp = (HashMap)realloc(
    *hashmap,
    sizeof(struct hashmap) + new_s_buckets * sizeof(struct hashmap_node)
  );
  assert(temp != NULL);
  temp->s_buckets = new_s_buckets;

  for (uint64_t i = 0; i < (*hashmap)->s_buckets; i++) {
    HashMapNode bucket = &(temp->buckets[hash(head->key, temp->s_buckets)]);
    head = &((*hashmap)->buckets[i]); // defined above

    if (bucket->key != NULL) {
      HashMapNode *bucket_curr = &(bucket->next);
      for (; *bucket_curr != NULL; *bucket_curr = (*bucket_curr)->next);

      HashMapNode node = (HashMapNode)calloc(1, sizeof(struct hashmap_node));
      assert(node != NULL);

      node->key    = head->key;
      node->value  = head->value;
      node->next   = NULL;
      *bucket_curr = node;
    } else {
      bucket->key   = head->key;
      bucket->value = head->value;
      bucket->next  = NULL;
    }

    HashMapNode curr = head->next;
    while (curr != NULL) {
      curr = curr->next;
      HashMapNode node = curr;

      bucket = &((*hashmap)->buckets[hash(node->key, temp->s_buckets)]);
      if (bucket->key == NULL) {
        bucket->key   = node->key;
        bucket->value = node->value;
        free(node);
        continue;
      }

      HashMapNode *bucket_curr = &(bucket->next);
      for (; *bucket_curr != NULL; *bucket_curr = (*bucket_curr)->next);
      *bucket_curr = node;
    }
  }

  (*hashmap) = temp;
  return true;
}

void* hashmap_get(HashMap hashmap, char *key) {
  if (hashmap == NULL)
    return NULL;
  if (key == NULL)
    return NULL;

  uint64_t index = hash(key, hashmap->s_buckets);
  if (hashmap->buckets[index].key == NULL)
    return NULL;

  if (strcmp(hashmap->buckets[index].key, key) == 0)
    return hashmap->buckets[index].value;

  HashMapNode current = hashmap->buckets[index].next;
  for (; current != NULL; current = current->next)
    if (strcmp(current->key, key) == 0)
      return current->value;

  return NULL;
}

bool hashmap_remove(HashMap hashmap, char *key, void free_value(void *value), const bool to_free) {
  if (hashmap == NULL)
    return false;
  if (key == NULL)
    return false;

  uint64_t index   = hash(key, hashmap->s_buckets);
  HashMapNode head = &(hashmap->buckets[index]);

  while (head->key != NULL && strcmp(head->key, key) == 0) {
    if (free_value && to_free) {
      free_value(head->value);
    } else if (to_free) {
      free(head->value);
    }
    free(head->key);

    if (head->next == NULL) {
      head->key   = NULL;
      head->value = NULL;
      head->next  = NULL;
      return true;
    }

    head->key   = (head->next)->key;
    head->value = (head->next)->value;
    head->next  = (head->next)->next;
  }

  HashMapNode *current = &(hashmap->buckets[index].next);
  for (; *current != NULL && strcmp((*current)->key, key) != 0; current = &((*current)->next));
  if (*current == NULL)
    return false;

  HashMapNode delete = *current;
  *current = (*current)->next;

  if (free_value && to_free) {
    free_value(delete->value);
  } else if (to_free) {
    free(delete->value);
  }
  free(delete->key);
  free(delete);
  hashmap->s_elements--;

  return true;
}

bool hashmap_free(HashMap hashmap, void free_value(void *value), const bool to_free) {
  if (hashmap == NULL)
    return false;
  for (uint64_t i = 0; i < hashmap->s_buckets; i++) {
    if (!hashmap->buckets[i].key)
      continue;

    free(hashmap->buckets[i].key);
    if (hashmap->buckets[i].value && to_free) {
      if (free_value)
        free_value(hashmap->buckets[i].value);
      else
        free(hashmap->buckets[i].value);
    } 

    HashMapNode current = hashmap->buckets[i].next;
    while (current != NULL) {
      HashMapNode node = current;
      current = current->next;
      if (free_value && to_free) {
        free_value(node->value);
      } else if (to_free) {
        free(node->value);
      }
      free(node->key);
      free(node);
    }
  }
  free(hashmap);
  return true;
}

// ==================================================# PRIVATE #=================================================================

bool hashmap_near_capacity(HashMap hashmap) {
  assert(hashmap != NULL);
  assert(hashmap->s_buckets != 0);
  return (float)hashmap->s_elements / hashmap->s_buckets >= hashmap->load_threshold_factor;
}

uint64_t hashmap_next_power_2(uint64_t n) {
    assert(n > 0);
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

uint64_t hash(char *key, uint64_t s_buckets) {
  assert(key != NULL);
  assert(s_buckets != 0);

  uint64_t len = strlen(key);
  uint64_t hash_value = 0;
  for (uint64_t i = 0; i < len; i++)
    hash_value = hash_value * 31 + key[i];

  if (len >= 2) {
    uint32_t a = (uint32_t)(key[0] + key[1]) << 16,
             b = key[len - 2] + key[len - 1];
    hash_value ^= (a | b);
  }

  return hash_value & (s_buckets - 1);
}
