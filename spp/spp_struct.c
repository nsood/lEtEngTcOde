#include <linux/kernel.h>
#include <linux/slab.h>

#define SPP_HEAP_MAX 8192
#define SPP_HASH_MAX 8192

struct spp_heap{
	int (*cmp)(void *,void *);
	void **heap;
	unsigned int size,pos;
};
struct spp_heap *spp_heap_new(unsigned int size,
					int (*cmp)(void*,void *))
{
	struct spp_heap *h = NULL;
	if (0 == size 
		|| size > SPP_HEAP_MAX 
		|| NULL == cmp)
		return h;
	h = kmalloc(sizeof(*h),GFP_KERNEL);
	if (h == NULL)
		return h;
	h->heap = kmalloc(sizeof(void *) * size,GFP_KERNEL);
	if (h->heap == NULL) {
		kfree(h);
		return h;
	}

	h->size = size;
	h->cmp = cmp;
	h->pos = 1;
	return h;
}
unsigned int spp_heap_size(struct spp_heap *h)
{
	return h->size;
}
unsigned int spp_heap_pos(struct spp_heap *h)
{
	return h->pos;
}

bool spp_heap_empty(struct spp_heap *h)
{
	return h->pos == 1;
}

void *spp_heap_top(struct spp_heap *h)
{
	return h->heap[1];		
}

void *spp_heap_del(struct spp_heap *h)
{
	void *res = NULL,*last;
	unsigned int pos;
	
	res = h->heap[1];
	last = h->heap[--h->pos];	
	for (pos = 1; (pos << 1) < h->pos ; ) {
		unsigned int left,right,min;
		left = pos << 1;
		right = left + 1;
		int ret = h->cmp(h->heap[left],h->heap[right]);
		if (ret < 0) 
			min = left;
		else 
			min = right;
		
		if (h->cmp(last,h->heap[min]) < 0) 
			break;
		
		h->heap[pos]  = h->heap[min];
		pos = min;
	}	
	h->heap[pos] = last;
	return res;
}

void spp_heap_visit(struct spp_heap *h,void (*visit)(void *))
{
	unsigned int i;
	for (i = 1 ; i < h->pos ; i++)
		visit(h->heap[i]);
}

bool spp_heap_add(struct spp_heap *h,void *v)
{
	void **heap;
	if (h->pos >= h->size) {
		 if (h->size >= SPP_HEAP_MAX)
		 	return false;
		 
		h->size <<= 1;
		heap = krealloc(h->heap,sizeof(void *) * h->size,GFP_ATOMIC);
		if (heap == NULL) {
			h->size >>= 1;
			return false;
		} else {
			h->heap = heap;
		}
	}

	heap = h->heap;	
	
	unsigned int pos = h->pos;
	while (pos > 1) {
		unsigned int parent = pos >> 1;
		int ret  = h->cmp(heap[parent], v);
		if (ret < 0)
			break;
		heap[pos] = heap[parent];		
		pos = parent;
	}

	heap[pos] = v;
	h->pos++;
	return true;
}


void spp_heap_destroy(struct spp_heap *h)
{
	if (h->heap != NULL)
		kfree(h->heap);
	kfree(h);
}

struct spp_hash_table {
	bool (*equ)(void *,void *);
	void *(*new)(void *);
	void (*destroy)(struct hlist_node *);
	void *(*entry_get)(struct hlist_node *);
	struct hlist_node *(*hnode_get)(void *);

	u32 size,entries;
	struct hlist_head head[0];
};


struct spp_hash_table *spp_hash_table_new(
					u32 size,
					bool (*equ)(void *,void *),
					void *(*new)(void *),
					void (*destroy)(struct hlist_node *),
					void *(*entry_get)(struct hlist_node *),
					struct hlist_node *(*hnode_get)(void *))
{
	struct spp_hash_table *ht = NULL;
	if (equ == NULL ||new == NULL || destroy == NULL || entry_get == NULL || hnode_get == NULL) 
		goto out;
	
	ht = kmalloc(sizeof(*ht) + sizeof(struct hlist_head) * size,
										GFP_KERNEL);

	if (ht == NULL)
		goto out;
	
	ht->equ = equ;
	ht->new = new;
	ht->destroy = destroy;
	ht->entry_get = entry_get;
	ht->hnode_get = hnode_get;
	ht->size = size;
	ht->entries = 0;
	
	u32 i;
	for (i = 0 ; i < size ; i++) 
		INIT_HLIST_HEAD(&ht->head[i]);
	
out:	
	return ht;
}

void spp_hash_table_free(struct spp_hash_table *ht)
{
	kfree(ht);
}

void spp_hash_table_destroy(struct spp_hash_table *ht)
{
	u32 i;
	struct hlist_node *ent,*_next;

	for (i = 0 ; i < ht->size ; i++) {
		if (hlist_empty(&ht->head[i]))
			continue;
		
		hlist_for_each_safe(ent,_next,&ht->head[i]) {
			hlist_del(ent);
			ht->destroy(ent);
		}
	}
}
int spp_hash_add(struct spp_hash_table *ht,u32 hash,void *v,void **ptr)
{
	struct hlist_node *pos;
	void *node;
//	if (ht->entries >= SPP_HASH_MAX)
//		return -ENOMEM;
	
	hash %= ht->size;
	hlist_for_each(pos,&ht->head[hash]) {
		node = ht->entry_get(pos);
		if (ht->equ(node,v))
			return -EEXIST;
	}

	node = ht->new(v);
	if (node == NULL)
		return -ENOMEM;

	hlist_add_head(ht->hnode_get(node),&ht->head[hash]);

	ht->entries++;
	
	if (ptr != NULL)
		*ptr = node;
	return 0;
}

void *spp_hash_get(struct spp_hash_table *ht,u32 hash,void *v,bool del)
{	
	void *res = NULL;
	struct hlist_node *pos,*_next;
	hash %= ht->size;
	hlist_for_each_safe(pos,_next,&ht->head[hash]) {
		if (ht->equ(pos,v)) {
			res = ht->entry_get(pos);
			if (del) {
				hlist_del(pos);
				ht->destroy(pos);
				res = NULL;
			}
			break;
		}
	}
	return res;
}

void spp_hash_visit(struct spp_hash_table *ht,void (*visit)(void *))
{
	u32 i;
	for (i = 0 ; i < ht->size ; i++) {
		struct hlist_node *pos,*_next;
		hlist_for_each_safe(pos,_next,&ht->head[i]) {
			visit(ht->entry_get(pos));
		}
	}
}
