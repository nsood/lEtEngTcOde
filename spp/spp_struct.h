#ifndef _SPP_STRUCT_H
#define _SPP_STRUCT_H

#include <linux/list.h>

struct spp_heap;
struct spp_heap *spp_heap_new(unsigned int size,
					int (*cmp)(void*,void *));
void *spp_heap_top(struct spp_heap *h);
void *spp_heap_del(struct spp_heap *h);
bool spp_heap_add(struct spp_heap *h,void *v);
void spp_heap_destroy(struct spp_heap *h);
bool spp_heap_empty(struct spp_heap *h);
void spp_heap_visit(struct spp_heap *h,void (*visit)(void *));
unsigned int spp_heap_size(struct spp_heap *h);
unsigned int spp_heap_pos(struct spp_heap *h);

struct spp_hash_table;
void *spp_hash_get(struct spp_hash_table *ht,u32 hash,void *v,bool del);
int spp_hash_add(struct spp_hash_table *ht,u32 hash,void *v,void **ptr);
void spp_hash_table_destroy(struct spp_hash_table *ht);
struct spp_hash_table *spp_hash_table_new(
						u32 size,
						bool (*equ)(void *,void *),
						void *(*new)(void *),
						void (*destroy)(struct hlist_node *),
						void *(*entry_get)(struct hlist_node *),
						struct hlist_node *(*hnode_get)(void *));
void spp_hash_table_free(struct spp_hash_table *ht);
void spp_hash_visit(struct spp_hash_table *ht,void (*visit)(void *));


#endif
