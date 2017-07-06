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

#define SPP_FLAG_MASK 0x3
#define SPP_PACKET_MAX 512

struct spp_packet_entry {
	struct hlist_node next;
	u32 saddr,daddr;
	u16 sport,dport;
	u8 protocol;
	u16 ulen;
	u32 uhash;
};

struct spp_network_ctx{
	struct kmem_cache *pkt_cache,*dup_cache;
	u32 hrand;
	spinlock_t lock;
	struct spp_hash_table *pkt;
};

struct spp_dumper{
	struct sk_buff *skb;
	u16 cnt;
};

static struct spp_network_ctx nctx;

static bool spp_packet_hash(const struct iphdr *iph,struct spp_packet_entry *he)
{
	u16 iphlen = iph->ihl << 2;
	u8 *data = NULL;
	struct udphdr *uh;
	struct tcphdr *th;
	if (!(iph->frag_off & IP_DF))
		return false;
	
	u8 *transport = (u8 *)iph + iphlen;
	u16 translen = ntohs(iph->tot_len) - iphlen;
	switch (iph->protocol) {
	case IPPROTO_TCP:
		th = (struct tcphdr *)transport; 
		data = transport + (th->doff << 2);
		he->ulen = translen - (th->doff << 2);
		
		he->sport = th->source;
		he->dport = th->dest;
	break;
	case IPPROTO_UDP:
		uh = (struct udphdr *)transport;
		he->ulen = ntohs(uh->len);
		data = (u8 *)uh + 1;
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

#if 0
	printk("HE %u.%u.%u.%u:%u to %u.%u.%u.%u:%u %u bytes %s\n",
		(he->saddr >> 24)&0xFF,(he->saddr >> 16)&0xFF,(he->saddr >> 8)&0xFF,he->saddr&0xFF,he->sport,
		(he->daddr >> 24)&0xFF,(he->daddr >> 16)&0xFF,(he->daddr >> 8)&0xFF,he->daddr&0xFF,he->dport,
		he->ulen,he->protocol == IPPROTO_TCP ? "TCP":"UDP");
#endif
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
	if (ent != NULL) 
		memcpy(ent,v,sizeof(*ent));
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
	if (dup && NULL != (dumper->skb = pskb_copy(skb,GFP_ATOMIC))) {
		dumper->cnt++;
		if (!spp_timer_new(nf_network_dump,dumper,nf_network_cancel,dup))
			nf_network_cancel(dumper);
	} else {
		kmem_cache_free(nctx.dup_cache,dumper);
	}
	BUG_ON(skb->data != (unsigned char *)ip_hdr(skb));	
	ip_local_out(skb);
}

static void pkt_timeout(void *v)
{
	struct spp_network_ctx *ctx = &nctx;
	struct spp_packet_entry *ent = (struct spp_packet_entry *)v;
	
	spin_lock_bh(&ctx->lock);
	hlist_del(&ent->next);
	spin_unlock_bh(&ctx->lock);

	kmem_cache_free(nctx.pkt_cache,ent);
}

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
#if 0
	struct ethhdr *eth;
#endif
	struct iphdr *iph;
	
	struct spp_network_ctx *ctx = &nctx;
	u16 trim = spp_trim_get();
	if (!trim)
		return NF_ACCEPT;
#if 0	
	eth  = (struct ethhdr *)skb_mac_header(skb);
	if (eth == NULL || eth->h_proto != ETH_P_IP || in != spp_lan()) 
		return NF_ACCEPT;
#endif	
	iph  = (struct iphdr *)skb_network_header(skb);
	if (iph == NULL || !spp_cip_match(iph->saddr,false) || !spp_sip_match(iph->daddr,false))
		return NF_ACCEPT;
	
	struct spp_packet_entry pe,*ppe = NULL;
	if (!spp_packet_hash(iph,&pe))
		return NF_ACCEPT;

	spin_lock_bh(&ctx->lock);
	int exsit = spp_hash_add(ctx->pkt,pe.uhash,&pe,(void **)&ppe);
	spin_unlock_bh(&ctx->lock);

	if (exsit == -EEXIST) 
		return NF_DROP;
	
	if (ppe != NULL && !spp_timer_new(pkt_timeout,ppe,NULL,spp_trim_get())) 
		pkt_timeout(ppe);
	
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
#if 0
	struct ethhdr *eth;
#endif
	struct iphdr *iph;
	struct sk_buff *new_skb;
	struct spp_dumper *dumper;
	struct spp_network_ctx *ctx = &nctx;
	
	u16 dup = spp_dup_get(0);
	if (!spp_wmm_get() && !dup)
		return NF_ACCEPT;
#if 0
	eth  = (struct ethhdr *)skb_mac_header(skb);
	printk("%s %d eth = %p %s lan_dev = %p out = %p\n",__func__,__LINE__,eth,out == spp_lan() ? "true":"false",spp_lan(),out);	
	if (eth == NULL || eth->h_proto != ETH_P_IP || out != spp_lan())
		return NF_ACCEPT;
#endif
	iph  = (struct iphdr *)skb_network_header(skb);
	if (iph == NULL || iph->tos & SPP_FLAG_MASK) {
		return NF_ACCEPT;
	}

	if (!spp_sip_match(iph->saddr,false)||!spp_cip_match(iph->daddr,false)) 
		return NF_ACCEPT;
	
	if (spp_wmm_get()) 
		iph->tos = 224;

	if (dup) 
		iph->tos |=0x1;	
	ip_send_check(iph);

	
	if (dup) {
		dumper = kmem_cache_alloc(ctx->dup_cache,GFP_ATOMIC);
		if (dumper == NULL)
			return NF_ACCEPT;
		new_skb = pskb_copy(skb, GFP_ATOMIC);
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
	spin_lock_init(&ctx->lock);
	
	if (nf_register_hooks(ops,sizeof(ops)/sizeof(ops[0])))
		goto fail;
	
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
	
	spin_lock_bh(&ctx->lock);
	spp_hash_table_destroy(ctx->pkt);
	spin_unlock_bh(&ctx->lock);
	spp_hash_table_free(ctx->pkt);
	
	kmem_cache_destroy(ctx->dup_cache);
	kmem_cache_destroy(ctx->pkt_cache);
}
