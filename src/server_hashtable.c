#include "server_hashtable.h"

#include <stdlib.h>
#include <string.h>

#define MAX(x,y) (x > y ? x : y)

size_t hash_key(char *);
struct server_table_pair *create_pair(char *, void *(*)(request*, response*));
bool resize_hash_table(server_hashtable *, size_t);
void free_pair(struct server_table_pair *);

server_hashtable *hashtable_create()
{
    server_hashtable *ht = malloc(sizeof(server_hashtable));
    if (ht == NULL) return NULL;

    ht->buckets = calloc(INITIAL_BUCKETS_SIZE, sizeof(struct server_table_pair));
    if (ht->buckets == NULL)
    {
        free(ht);
        return NULL;
    }

    ht->buckets_size = INITIAL_BUCKETS_SIZE;
    ht->pair_num = 0;

    return ht;
}

bool hashtable_put(server_hashtable *ht, char *endpoint, void *(*hdl_func)(request*, response*))
{
    /* args checks are done by higher levels */

    if (strncmp("", endpoint, 1) == 0)
        return false;

    /* create new pair */
    struct server_table_pair *new_node = create_pair(endpoint, hdl_func);
    if (new_node == NULL) return false;

    /* hash string to get correct bucket */
    size_t bucket_index = hash_key(endpoint) % ht->buckets_size;
    struct server_table_pair *node = *(ht->buckets + bucket_index);

    /* reach last pair of the bucket */
    if (node == NULL) // no pair in this bucket
    {
        *(ht->buckets + bucket_index) = new_node;
    }
    else
    {
        /* if key already exists, override value */
        /* otherwise just add the node           */
        struct server_table_pair *prev_node = NULL;
        while (node != NULL)
        {
            if (strncmp(node->endpoint, endpoint, MAX(strlen(endpoint), strlen(node->endpoint))) == 0)
            {
                node->hdl_func = hdl_func;
                return true;
            }
            prev_node = node;
            node = node->next;
        }
        prev_node->next = new_node;
        node = new_node;
    }
    ht->pair_num++;

    if ((double)ht->pair_num / ht->buckets_size >= 0.75)
        if (resize_hash_table(ht, (ht->buckets_size * 2) + 1) == false)
            return false;

    return true;
}

void *(*hashtable_get(server_hashtable *ht, char *key))(request*, response*)
{
    if (ht == NULL || key == NULL) return NULL;

    size_t bucket_index = hash_key(key) % ht->buckets_size;
    struct server_table_pair *pair = *(ht->buckets + bucket_index);
    if (pair == NULL) return NULL;

    while (pair->next != NULL)
    {
        if (strncmp(pair->endpoint, key, MAX(strlen(pair->endpoint), strlen(key))) == 0)
            return pair->hdl_func;

        pair = pair->next;
    }

    return pair->hdl_func;
}

void hashtable_destroy(server_hashtable *ht)
{
    if (ht == NULL) return;

    for (size_t i = 0; i < ht->buckets_size; i++)
    {
        struct server_table_pair *current = ht->buckets[i];
        struct server_table_pair *next_pair = NULL;

        while (current != NULL)
        {
            next_pair = current->next;
            free_pair(current);

            current = next_pair;
        }
    }

    free(ht->buckets);
    free(ht);
}

size_t hash_key(char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

struct server_table_pair *create_pair(char *key, void *(*val)(request*, response*))
{
    struct server_table_pair *node = malloc(sizeof(struct server_table_pair));
    if (node == NULL) return NULL;

    node->endpoint = malloc(strlen(key) + 1);
    if (node->endpoint == NULL) return NULL;

    strcpy(node->endpoint, key);
    node->hdl_func = val;
    node->next = NULL;

    return node;
}

bool resize_hash_table(server_hashtable *ht, size_t new_size)
{
    /* new array with updated size */
    struct server_table_pair **new_buckets = calloc(new_size, sizeof(struct server_table_pair));
    if (new_buckets == NULL) return false;

    /* reorder every pair */
    for (size_t i = 0; i < ht->buckets_size; i++)
    {
        struct server_table_pair *curr_node = ht->buckets[i];
        while (curr_node != NULL)
        {
            struct server_table_pair *next = curr_node->next;

            size_t new_index = hash_key(curr_node->endpoint) % new_size;
            curr_node->next = new_buckets[new_index];
            new_buckets[new_index] = curr_node;
            curr_node = next;
        }
    }

    /* free old array and link new one */
    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->buckets_size = new_size;

    return true;
}

void free_pair(struct server_table_pair *pair)
{
    free(pair->endpoint);
    free(pair);
}
