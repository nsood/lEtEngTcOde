#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include "spp_config.h"
#include "spp_struct.h"

#define SPP_SERVER_MAX  64
#define SPP_CLIENT_MAX  32

struct ip_hash_entry {
	struct hlist_node next;
	u32 ip;
};
struct spp_config_ctx{
	struct kmem_cache *cache;
	struct net_device *lan
#if 0
,*wan
#endif
;
	u16 dup[4];
	u16 trim;
	bool wmm;
	spinlock_t slock;
	struct spp_hash_table *ht_sip;
	spinlock_t clock;
	struct spp_hash_table *ht_cip;
 };

static struct spp_config_ctx cctx;

static bool ip_equ(void *v1,void *v2)
{
	struct ip_hash_entry *ipe = (struct ip_hash_entry *)v1;
	u32 u2 = (uintptr_t)v2 & 0xFFFFFFFF;
	return ipe->ip == u2;
}

static void *ip_new(void *v)
{
	struct ip_hash_entry *ent = kmem_cache_alloc(cctx.cache,GFP_ATOMIC);
	if (ent != NULL) 
		ent->ip = ((uintptr_t)v) & 0xFFFFFFFF;
	
	return ent;
}
static void ip_destroy(struct hlist_node *v)
{
	kmem_cache_free(cctx.cache,container_of(v,struct ip_hash_entry,next));
}

static void *ip_entry_get(struct hlist_node *node)
{
	return container_of(node,struct ip_hash_entry,next);
}

static struct hlist_node *ip_node_get(void *v)
{
	return &((struct ip_hash_entry *)v)->next;
}


int spp_sip_add(u32 ip)
{
	int res;
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->slock);
	res =  spp_hash_add(ctx->ht_sip,ip,(void *)(uintptr_t)ip,NULL);	
	spin_unlock(&ctx->slock);
	return res;
}

void spp_sip_del(u32 ip)
{
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->slock);
	spp_hash_get(ctx->ht_sip,ip,(void *)(uintptr_t)ip,true);	
	spin_unlock(&ctx->slock);
}

void spp_cip_del(u32 ip)
{
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->clock);
	spp_hash_get(ctx->ht_cip,ip,(void *)(uintptr_t)ip,true);	
	spin_unlock(&ctx->clock);
}

int spp_cip_add(u32 ip)
{
	int res;
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->clock);
	res =  spp_hash_add(ctx->ht_cip,ip,(void *)(uintptr_t)ip,NULL);	
	spin_unlock(&ctx->clock);
	return res;
}


bool spp_sip_match(u32 ip,bool del)
{
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->slock);
	void *ent = spp_hash_get(ctx->ht_sip,ip,(void *)(uintptr_t)ip,del);
	spin_unlock(&ctx->slock);
	return ent != NULL;
}

bool spp_cip_match(u32 ip,bool del)
{
	struct spp_config_ctx *ctx = &cctx;
	spin_lock(&ctx->clock);
	void *ent = spp_hash_get(ctx->ht_cip,ip,(void *)(uintptr_t)ip,del);
	spin_unlock(&ctx->clock);
	return ent != NULL;
}

static char *lan = "br-lan";
#if 0
static char *wan = "eth0";
#endif

module_param(lan,charp,0644);
#if 0
module_param(wan,charp,0644);
#endif
int  spp_config_init(void)
{
	struct spp_config_ctx *sc = &cctx;
	sc->lan =  __dev_get_by_name(&init_net,lan);
	if (sc->lan == NULL)
		goto fail;
#if 0
	sc->wan =  __dev_get_by_name(&init_net,wan);
	if (sc->wan == NULL)
		goto fail;
	printk("lan dev = %p wan dev = %p HZ = %u\n",sc->lan,sc->wan,HZ);
#endif

	sc->wmm = true;
	sc->dup[0] = 20;
	sc->dup[1] = 0;
	sc->dup[2] = 0;
	sc->trim = 100;

	sc->cache = kmem_cache_create("spp_ip_entry",
								    sizeof(struct ip_hash_entry),0,0,
								    NULL);
	if (sc->cache == NULL)
		goto fail;
	
	spin_lock_init(&sc->slock);
	sc->ht_sip = spp_hash_table_new(
									SPP_SERVER_MAX,
									ip_equ,
									ip_new,
									ip_destroy,
									ip_entry_get,
									ip_node_get);
	if (sc->ht_sip == NULL)
		goto fail;

	spin_lock_init(&sc->clock);
	sc->ht_cip = spp_hash_table_new(
									SPP_CLIENT_MAX,
									ip_equ,
									ip_new,
									ip_destroy,
									ip_entry_get,
									ip_node_get);

	if (sc->ht_cip == NULL)
		goto fail;

	return 0;
fail:
	if (sc->ht_cip) 
		spp_hash_table_free(sc->ht_cip);
	
	if (sc->ht_sip) 
		spp_hash_table_destroy(sc->ht_sip);
		
	if (sc->cache)
		kmem_cache_destroy(sc->cache);
	return -ENOMEM;
}

void  spp_config_fini(void)
{
	struct spp_config_ctx *sc = &cctx;

	spin_lock(&sc->clock);
	spp_hash_table_destroy(sc->ht_cip);
	spin_unlock(&sc->clock);
	spp_hash_table_free(sc->ht_cip);
	
	spin_lock(&sc->slock);
	spp_hash_table_destroy(sc->ht_sip);	
	spin_unlock(&sc->slock);
	spp_hash_table_free(sc->ht_sip);

	kmem_cache_destroy(sc->cache);
}

struct net_device *spp_lan(void){	return cctx.lan;}
bool spp_wmm_get(void){	return cctx.wmm;}
void spp_wmm_set(bool b){cctx.wmm = b;}
u16  spp_dup_get(u16 index){return cctx.dup[index&3];}
void spp_dup_set(u16 index,u16 val){cctx.dup[index&3] = val;}
u16 spp_trim_get(void){return cctx.trim;}
void spp_trim_set(u16 v){cctx.trim = v;}

