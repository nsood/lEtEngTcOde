#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

#include "spp_struct.h"

struct spp_timer{
	void (*func)(void *);
	void (*cancel)(void *);
	void *data;
	unsigned long expire,save;
};

struct spp_timer_ctx {
	struct work_struct work;
	struct timer_list timer;
	struct kmem_cache *cache;
	spinlock_t lock;
	struct spp_heap *heap;
};


static struct spp_timer_ctx tctx;

static void spp_timer_cb(struct work_struct *ws)
{
	struct spp_timer_ctx *ctx = container_of(ws,struct spp_timer_ctx,work);
	spin_lock(&ctx->lock);
	while (!spp_heap_empty(ctx->heap)) {
		struct spp_timer *tm =  (struct spp_timer *)spp_heap_top(ctx->heap);
		if (tm->expire > jiffies) 
			break;

		tm = spp_heap_del(ctx->heap);
		tm->func(tm->data);
		
		kmem_cache_free(ctx->cache,tm);
	}
	spin_unlock(&ctx->lock);
}
static void spp_timer_process(unsigned long data)
{
	struct spp_timer_ctx *ctx = (struct spp_timer_ctx *)data;
	schedule_work(&ctx->work);
	ctx->timer.expires +=  msecs_to_jiffies(1);
	add_timer(&ctx->timer);
}

static int spp_timer_cmp(void *v1,void *v2)
{
	struct spp_timer *t1 = (struct spp_timer *)v1;
	struct spp_timer *t2 = (struct spp_timer *)v2;

	return (int)(t1->expire - t2->expire);
}

static void spp_timer_cancel(void *v)
{
	struct spp_timer *tm = (struct spp_timer *)v;
	
	if (tm->cancel != NULL) 
		tm->cancel(tm->data);
	kmem_cache_free(tctx.cache,tm);
}

bool spp_timer_new(void (*func)(void *),void *v,void (*cancel)(void *),unsigned long delay)
{
	bool res;
	struct spp_timer_ctx *ctx = &tctx;
	struct spp_timer *tm = kmem_cache_alloc(ctx->cache,GFP_ATOMIC);
	if (tm == NULL)
		return false;

	tm->data = v;
	tm->func = func;
	tm->save = jiffies;
	tm->expire = jiffies + msecs_to_jiffies(delay);
	tm->cancel = cancel;
	
	spin_lock(&ctx->lock);
	res = spp_heap_add(ctx->heap,tm);
	spin_unlock(&ctx->lock);
	return res;
}
int  spp_timer_init(void)
{
	struct spp_timer_ctx *ctx = &tctx;
	ctx->cache =  kmem_cache_create("spp_timer",
					    sizeof(struct spp_timer), 0, 0,
					    NULL);
	

	
	if (ctx->cache == NULL) 
		return -ENOMEM;
	
	ctx->heap = spp_heap_new(1024,spp_timer_cmp);
	if (ctx->heap == NULL) {
		kmem_cache_destroy(ctx->cache);
		return -ENOMEM;
	}
	
	spin_lock_init(&ctx->lock);
	init_timer(&ctx->timer);
	ctx->timer.expires = jiffies +  msecs_to_jiffies(1);
	ctx->timer.function = spp_timer_process;
	ctx->timer.data = (unsigned long)ctx;

	INIT_WORK(&ctx->work,spp_timer_cb);
	add_timer(&ctx->timer);
	return 0;
}

void  spp_timer_fini(void)
{
	struct spp_timer_ctx *ctx = &tctx;

	del_timer(&ctx->timer);
	
	spin_lock(&ctx->lock);
	spp_heap_visit(ctx->heap,spp_timer_cancel);
	spin_unlock(&ctx->lock);
	
	spp_heap_destroy(ctx->heap);
	kmem_cache_destroy(ctx->cache);
}
