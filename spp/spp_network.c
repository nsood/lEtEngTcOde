#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <net/ip.h>

#include "spp_config.h"
#include "spp_timer.h"
#include "spp_struct.h"

//#define DEBUG_K
#ifdef DEBUG_K
#define debug_k(format,...) printk("%s(%d):\t\t"format"\n",__func__, __LINE__, ##__VA_ARGS__)
#else
#define debug_k(format,...)
#endif

#define SPP_FLAG_MASK 0x3
#define SPP_PACKET_MAX 1024

struct spp_packet_entry {
	struct hlist_node next;
#ifndef _USE_SPP_TIMER
	u32 expire;
#endif
	u32 saddr,daddr;
	u16 sport,dport;
	u8 protocol;
	u16 ulen;
	u32 uhash;
};

struct spp_network_ctx{
	struct kmem_cache *pkt_cache,
					  *dup_cache;

	u32 hrand;
	spinlock_t trim_lock;
	struct spp_hash_table *pkt;

#ifndef _USE_SPP_TIMER
	struct timer_list  trim_tm;
	struct work_struct trim_wk;


	spinlock_t dump_lock;
	u32 ndumper;
	struct list_head dump_gc;
	struct timer_list dump_tm;
	struct work_struct dump_wk;
#endif
};

struct spp_dumper{
#ifndef _USE_SPP_TIMER
	struct list_head gc;
	unsigned long expires;
#endif
	struct sk_buff *skb;
};

static struct spp_network_ctx nctx;

static bool spp_packet_hash(const struct iphdr *iph,struct spp_packet_entry *he)
{
	u16 iphlen = iph->ihl << 2;
	u8 *data = NULL;
	struct udphdr *uh;
	if (!(iph->frag_off & htons(IP_DF)))
		return false;
	
	u8 *transport = (u8 *)iph + iphlen;
	switch (iph->protocol) {
	case IPPROTO_UDP:
		uh = (struct udphdr *)transport;
		he->ulen = ntohs(uh->len) -sizeof(*uh);
		data = (u8 *)(uh + 1);
		he->sport = uh->source;
		he->dport = uh->dest;
	break;

	}
	
	if (he->ulen == 0 || data == NULL)
		return false;
	
	he->protocol = iph->protocol;

	he->uhash = jhash(data,he->ulen,nctx.hrand);
	he->saddr = iph->saddr;
	he->daddr = iph->daddr;

	return true;
}

static bool spp_packet_equ(void *v1,void *v2)
{
	struct spp_packet_entry *e1,*e2;
	e1 = (struct spp_packet_entry *)v1;
	e2 = (struct spp_packet_entry *)v2;
	return (e1->uhash == e2->uhash 
		    &&e1->ulen == e2->ulen
			&&e1->protocol == e2->protocol
			&&e1->saddr == e2->saddr 
			&&e1->daddr == e2->daddr
			&&e1->sport == e2->sport
			&&e1->dport == e2->dport);
}

static void *spp_packet_entry_get(struct hlist_node *node)
{
	return (void *)container_of(node,struct spp_packet_entry,next);
}
static void *spp_packet_new(void *v)
{
	struct spp_packet_entry *ent = kmem_cache_alloc(nctx.pkt_cache,GFP_ATOMIC);
	if (ent != NULL) {
		memcpy(ent,v,sizeof(*ent));
		ent->expire = jiffies + spp_trim_get();
	}
	return (void *)ent;
}

static void spp_packet_destroy(struct hlist_node *node)
{
	kmem_cache_free(nctx.pkt_cache,container_of(node,struct spp_packet_entry,next));
}

struct hlist_node *spp_packet_node_get(void *v)
{
	return &((struct spp_packet_entry *)v)->next;
}

#ifdef _USE_SPP_TIMER
static void nf_network_cancel(void *v)
{
	struct spp_dumper *sd = (struct spp_dumper *)v;
	kfree_skb(sd->skb);
	kmem_cache_free(nctx.dup_cache,sd);
}

static void nf_network_dump(void *v)
{
	struct spp_dumper *dumper = (struct spp_dumper *)v;
	struct sk_buff *skb = dumper->skb;
	u16 dup = spp_dup_get(dumper->cnt);
	if (dup && NULL != (dumper->skb = skb_copy(skb,GFP_ATOMIC))) {
		dumper->cnt++;
		if (!spp_timer_new(nf_network_dump,dumper,nf_network_cancel,dup))
			nf_network_cancel(dumper);
	} else {
		kmem_cache_free(nctx.dup_cache,dumper);
	}
	BUG_ON(skb->data != (unsigned char *)ip_hdr(skb));	
	ip_local_out(skb);
}

#else

static struct spp_dumper *spp_dumper_alloc(struct sk_buff *skb,u16 delay)
{
	struct spp_dumper *dumper = NULL;
	struct sk_buff *new_skb = NULL;
	
	struct spp_network_ctx *ctx = &nctx;
	spin_lock(&ctx->dump_lock);
	if (++ctx->ndumper >= 1024)
		goto out;	
	spin_unlock(&ctx->dump_lock);
#if 1	
	new_skb = pskb_copy(skb,GFP_ATOMIC);
	if (!new_skb) {
		spin_lock(&ctx->dump_lock);
		goto out;
	}
#endif	
	dumper = kmem_cache_alloc(ctx->dup_cache,GFP_ATOMIC);
	if (!dumper) {
		kfree_skb(new_skb);
		spin_lock(&ctx->dump_lock);
		goto out;
	}

	dumper->skb = new_skb;
	dumper->expires = jiffies + msecs_to_jiffies(delay);

	spin_lock(&ctx->dump_lock);
	list_add_tail(&dumper->gc,&ctx->dump_gc);
	spin_unlock(&ctx->dump_lock);

	return dumper;
out:
	ctx->ndumper--;
	spin_unlock(&ctx->dump_lock);
	return dumper;
}

static void pkt_timeout(void *v)
{
	struct spp_packet_entry *ent = (struct spp_packet_entry *)v;
	if (ent->expire <= jiffies) {
		hlist_del(&ent->next);
		kmem_cache_free(nctx.pkt_cache,ent);
	}
}

static void spp_trim_work_cb(struct work_struct *ws)
{
	struct spp_network_ctx *ctx = container_of(ws,struct spp_network_ctx,trim_wk);
	spin_lock(&ctx->trim_lock);
	spp_hash_visit(ctx->pkt,pkt_timeout);
	spin_unlock(&ctx->trim_lock);
}

static void spp_trim_timer_cb(unsigned long v)
{
	struct spp_network_ctx *ctx = (struct spp_network_ctx *)v;
	schedule_work(&ctx->trim_wk);
	
	ctx->trim_tm.expires = jiffies + msecs_to_jiffies(1000);	
	add_timer(&ctx->trim_tm);	
}

static void spp_dump_work_cb(struct work_struct *ws)
{
	struct spp_network_ctx *ctx = container_of(ws,struct spp_network_ctx,dump_wk);
	struct spp_dumper *pos,*_next;
	
	spin_lock(&ctx->dump_lock);
	list_for_each_entry_safe(pos,_next,&ctx->dump_gc,gc) {
		if (pos->expires > jiffies)
			break;
		list_del(&pos->gc);
		ctx->ndumper--;
		ip_local_out(pos->skb);
		kmem_cache_free(ctx->dup_cache,pos);
	}
	spin_unlock(&ctx->dump_lock);
}

static void spp_dump_timer_cb(unsigned long v)
{
	struct spp_network_ctx *ctx = (struct spp_network_ctx *)v;
	schedule_work(&ctx->dump_wk);
	ctx->dump_tm.expires++;
	add_timer(&ctx->dump_tm);
}

#endif

static unsigned int nf_network_up(
#if 0
							const struct nf_hook_ops *ops,
#else
							unsigned int hooknum,
#endif
                            struct sk_buff *skb,
                            const struct net_device *in,
                            const struct net_device *out,
                            int (*okfn)(struct sk_buff *))
{	
	struct iphdr *iph;
	
	struct spp_network_ctx *ctx = &nctx;
	u16 trim = spp_trim_get();
	if (!trim)
		return NF_ACCEPT;
	
	iph  = (struct iphdr *)skb_network_header(skb);
	if (iph == NULL || iph->protocol != IPPROTO_UDP)
		return NF_ACCEPT;

	if (!spp_cip_match(iph->saddr,false) || !spp_sip_match(iph->daddr,false))
		return NF_ACCEPT;

	struct spp_packet_entry pe,*ppe = NULL;
	if (!spp_packet_hash(iph,&pe))
		return NF_ACCEPT;

	spin_lock(&ctx->trim_lock);
	int exsit = spp_hash_add(ctx->pkt,pe.uhash,&pe,(void **)&ppe);
	spin_unlock(&ctx->trim_lock);

	if (exsit == -EEXIST) {
#ifdef DEBUG_K
		unsigned char *p = skb->data+ip_hdr(skb)->ihl * 4+sizeof(struct udphdr);
		debug_k("drop skb num : %02X %02X",*(p+12),*(p+13));
#endif
		return NF_DROP;
	}

	if (iph->tos) {
		iph->tos = 0;
		ip_send_check(iph);
	}
	return NF_ACCEPT;
}


static unsigned int nf_network_down(
#if 0
									const struct nf_hook_ops *ops,
#else
									unsigned int hooknum,
#endif
	                                struct sk_buff *skb,
	                                const struct net_device *in,
	                                const struct net_device *out,
	                                int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph;	
	u16 dup = spp_dup_get(0);
	if (!spp_wmm_get() && !dup)
		return NF_ACCEPT;

	iph  = (struct iphdr *)skb_network_header(skb);
	if (iph == NULL || iph->protocol != IPPROTO_UDP || iph->tos & SPP_FLAG_MASK) 
		return NF_ACCEPT;

	if (!spp_sip_match(iph->saddr,false)||!spp_cip_match(iph->daddr,false)) 
		return NF_ACCEPT;

	if (spp_wmm_get()) 
		iph->tos = 224;

	if (dup) 
		iph->tos |=0x1;	
	ip_send_check(iph);

	if (dup) {
#if 1
		spp_dumper_alloc(skb,dup);
#else		
		struct sk_buff *new_skb;
		struct spp_dumper *dumper;
		struct spp_network_ctx *ctx = &nctx;
		dumper = kmem_cache_alloc(ctx->dup_cache,GFP_ATOMIC);
		if (dumper == NULL)
			return NF_ACCEPT;
		new_skb = skb_copy(skb, GFP_ATOMIC);
		if (new_skb == NULL) {
			kmem_cache_free(ctx->dup_cache,dumper);
			return NF_ACCEPT;
		}

		dumper->cnt = 1;
		dumper->skb = new_skb;
		
		if (!spp_timer_new(nf_network_dump,dumper,nf_network_cancel,dup)) {
			kfree_skb(new_skb);
			kmem_cache_free(ctx->dup_cache,dumper);
		}
#endif	
	}
	return NF_ACCEPT;
}


static struct nf_hook_ops ops[2] = {
		[0] = {
	        .owner = THIS_MODULE,
	        .hook = nf_network_up,
	        .pf = NFPROTO_IPV4,
	        .hooknum = NF_INET_PRE_ROUTING,
	        .priority = -1,
        },
		[1] = { 
	        .owner = THIS_MODULE,
	        .hook = nf_network_down,
	        .pf = NFPROTO_IPV4,
	        .hooknum = NF_INET_POST_ROUTING,
	        .priority = -1,
        }
};

int  spp_network_init(void)
{
	struct spp_network_ctx *ctx = &nctx;


	ctx->ndumper = 0;
	
	get_random_bytes(&ctx->hrand,sizeof(ctx->hrand));
	ctx->pkt_cache = kmem_cache_create("spp_packet",
									sizeof(struct spp_packet_entry),
									0,0,NULL);
	if (ctx->pkt_cache == NULL)
		goto fail;
	ctx->dup_cache = kmem_cache_create("spp_dumper",
									sizeof(struct spp_dumper),
									0,0,NULL);
	if (ctx->dup_cache == NULL)
		goto fail;
	
	ctx->pkt = spp_hash_table_new(SPP_PACKET_MAX,
								spp_packet_equ,
								spp_packet_new,
								spp_packet_destroy,
								spp_packet_entry_get,
								spp_packet_node_get);
	if (ctx->pkt == NULL)
		goto fail;
	spin_lock_init(&ctx->trim_lock);
	
	if (nf_register_hooks(ops,sizeof(ops)/sizeof(ops[0])))
		goto fail;
	
#ifndef _USE_SPP_TIMER
	init_timer(&ctx->trim_tm);
	ctx->trim_tm.expires = jiffies + msecs_to_jiffies(1000);
	ctx->trim_tm.function = spp_trim_timer_cb;
	ctx->trim_tm.data = (unsigned long)ctx;
	INIT_WORK(&ctx->trim_wk,spp_trim_work_cb);
	add_timer(&ctx->trim_tm);
	
	INIT_LIST_HEAD(&ctx->dump_gc);
	spin_lock_init(&ctx->dump_lock);
	
	init_timer(&ctx->dump_tm);
	ctx->dump_tm.expires = jiffies + 1;
	ctx->dump_tm.function = spp_dump_timer_cb;
	ctx->dump_tm.data = (unsigned long)ctx;
	INIT_WORK(&ctx->dump_wk,spp_dump_work_cb);	
	add_timer(&ctx->dump_tm);	
#endif

	return 0;
fail:
	if (ctx->pkt) 
		spp_hash_table_free(ctx->pkt);

	if (ctx->dup_cache != NULL)
		kmem_cache_destroy(ctx->dup_cache);
	if (ctx->pkt_cache != NULL)
		kmem_cache_destroy(ctx->pkt_cache);
	return -ENOMEM;
}

void spp_network_stop(void)
{
	nf_unregister_hooks(ops,sizeof(ops)/sizeof(ops[0]));
}
void  spp_network_fini(void)
{
	struct spp_network_ctx *ctx = &nctx;
	
#ifndef _USE_SPP_TIMER	
	del_timer(&ctx->trim_tm);
	del_timer(&ctx->dump_tm);
	
	struct spp_dumper *pos,*_next;
	list_for_each_entry_safe(pos,_next,&ctx->dump_gc,gc) {
		list_del(&pos->gc);
		kfree_skb(pos->skb);
		kmem_cache_free(ctx->dup_cache,pos);
	}
#endif

	spin_lock(&ctx->trim_lock);
	spp_hash_table_destroy(ctx->pkt);
	spin_unlock(&ctx->trim_lock);
	spp_hash_table_free(ctx->pkt);
	
	kmem_cache_destroy(ctx->dup_cache);
	kmem_cache_destroy(ctx->pkt_cache);
}
