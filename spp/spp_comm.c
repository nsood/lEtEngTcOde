#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include "spp_comm.h"
#include "spp_config.h"

/*
* 处理与应用层的通信
*
*/
    
struct sock *nl_sk = NULL;

//向用户态进程回发消息
void spp_nl_send_msg(u8 *data, u16 data_len, int pid)
{
    struct  sk_buff *skb;
    struct  nlmsghdr *nlh;
    int     len = NLMSG_SPACE(data_len);
    u8      *old_tail;
    
    if(!data || !nl_sk)
    {
        return ;
    }
    printk(KERN_ERR "pid:%d\n",pid);
    skb = alloc_skb(len, GFP_KERNEL);
    if(!skb)
    {
        printk(KERN_ERR "my_net_link:alloc_skb error\n");
    }

    old_tail = skb->tail;

    nlh = nlmsg_put(skb, 0, 0, 0, len - sizeof(struct nlmsghdr), 0);
    memcpy(NLMSG_DATA(nlh), data, data_len);
    nlh->nlmsg_len = skb->tail - old_tail;

    nlh->nlmsg_pid = 0;
    nlh->nlmsg_flags = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
    NETLINK_CB(skb).portid = pid;
#else
    NETLINK_CB(skb).pid = pid;
#endif
    NETLINK_CB(skb).dst_group = 0;
    printk("send message '%hu'.\n", data_len);
    
    netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
}


void spp_comm_msg_process(struct nlmsghdr *nlh)
{
    spp_comm_msg_t *p_msg;
    int pid;
    if (nlh == NULL)
        return;
    
    pid = nlh->nlmsg_pid;
    p_msg = (spp_comm_msg_t *)NLMSG_DATA(nlh);
    
    printk("Message received pid:%d, type:%hu\n", pid, nlh->nlmsg_type);
    
    switch(p_msg->msg_type)
    {
        case SPP_COMM_SIP_ADD:
            spp_sip_add(*(u32 *)&p_msg->data[0]);
            break;
        case SPP_COMM_SIP_DEL:
            spp_sip_del(*(u32 *)&p_msg->data[0]);
            break;
        case SPP_COMM_CIP_ADD:
            spp_cip_add(*(u32 *)&p_msg->data[0]);
            break;
        case SPP_COMM_CIP_DEL:
            spp_cip_del(*(u32 *)&p_msg->data[0]);
            break;
            
        case SPP_COMM_WMM_GET:
        {
            spp_comm_msg_t state;
            spp_comm_u16_t *p_value = (spp_comm_u16_t *)&state.data[0];
            
            state.msg_type = SPP_COMM_WMM_GET_ACK;
            p_value->value = spp_wmm_get();
            
            spp_nl_send_msg((u8 *)&state, sizeof(state), pid);
            break;
        }
        case SPP_COMM_WMM_SET:
        {
            spp_comm_u16_t *p_value = (spp_comm_u16_t *)&p_msg->data[0];
            
            spp_wmm_set(p_value->value > 0 ? true : false);
            break;
        }
        
        case SPP_COMM_DUP_GET:
        {
            spp_comm_msg_t state;
            spp_comm_u16_t *p_value = (spp_comm_u16_t *)&state.data[0];
            spp_comm_u16_t *p_get = (spp_comm_u16_t *)&p_msg->data[0];
            
            state.msg_type = SPP_COMM_DUP_GET_ACK;
            p_value->value = spp_dup_get(p_get->value);
            
            spp_nl_send_msg((u8 *)&state, sizeof(state), pid);
            break;
        }
        case SPP_COMM_DUP_SET:
        {
            spp_comm_dump_set_t *p_value = (spp_comm_dump_set_t *)&p_msg->data[0];
            
            spp_dup_set(p_value->index, p_value->value);
            break;
        }
        
        case SPP_COMM_TRIM_GET:
        {
            spp_comm_msg_t state;
            spp_comm_u16_t *p_value = (spp_comm_u16_t *)&state.data[0];
            
            state.msg_type = SPP_COMM_TRIM_GET_ACK;
            p_value->value = spp_trim_get();
            
            spp_nl_send_msg((u8 *)&state, sizeof(state), pid);
            break;
        }
        case SPP_COMM_TRIM_SET:
        {
            spp_comm_u16_t *p_value = (spp_comm_u16_t *)&p_msg->data[0];
            
            spp_trim_set(p_value->value > 0 ? true : false);
            break;
        }
        default:
            break;
    }
}

void spp_nl_recv_msg(struct sk_buff *__skb)
{
    struct sk_buff *skb;
    
    printk("begin spp_nl_recv_msg\n");
    skb = skb_get (__skb);
    if(skb->len >= NLMSG_SPACE(0))
    {
        spp_comm_msg_process(nlmsg_hdr(skb));
    }
    kfree_skb(skb);
    
    printk("end spp_nl_recv_msg\n");
}

int spp_comm_init(void)
{ 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
    struct netlink_kernel_cfg cfg = {
        .input = spp_nl_recv_msg
    };
    
    nl_sk = netlink_kernel_create(&init_net, SPP_NETLINK_PROTO, &cfg);
#else
	nl_sk = netlink_kernel_create(&init_net,SPP_NETLINK_PROTO,0,spp_nl_recv_msg,NULL,THIS_MODULE);
#endif
    if(!nl_sk)
    {
        printk(KERN_ERR "create netlink socket error.\n");
        return 1;
    }
    printk("create netlink socket ok.\n");
    return 0;

}

void spp_comm_fini(void)
{
    if(nl_sk != NULL){
        sock_release(nl_sk->sk_socket);
    }
    printk("spp_netlink_exit\n");
}


