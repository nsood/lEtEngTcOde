#include "nl.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

int spp_netlink_connect(int protocol)
{
	struct sockaddr_nl src_addr;
	int sock_fd, retval;
	sock_fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if(sock_fd == -1)
	{
		printf("error getting socket: %s", strerror(errno));
		return -1;
	}
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;
	retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
	if(retval < 0)
	{
		printf("bind failed: %s", strerror(errno));
		close(sock_fd);
		return -1;
	}    
	return sock_fd;
}

void spp_netlink_close(int fd)
{
	close(fd);
}

int spp_netlink_send_msg(int nl_fd, u8 *data, u16 data_len)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	int state_smg = 0;
	
    int nlmsg_len = NLMSG_SPACE(data_len);
	nlh = (struct nlmsghdr *)malloc(nlmsg_len);
	if(!nlh){
		printf("malloc nlmsghdr error!\n");
		return -1;
	}
	
	memset(&dest_addr,0,sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;
    
	nlh->nlmsg_len = nlmsg_len;
	nlh->nlmsg_type = 0;
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(nlh), data, data_len);
    
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlmsg_len;
	//Create mssage
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	//send message
	printf("send smg\n");
    
	state_smg = sendmsg(nl_fd, &msg, 0);
    free(nlh);
    
	if(state_smg == -1)
	{
		printf("get error sendmsg = %s\n", strerror(errno));
        return -1;
	}

    return 0;
}

int spp_sip_add(int nl_fd, u32 ip)
{
    spp_comm_msg_t msg;
    spp_comm_u32_t *p_value = (spp_comm_u32_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_SIP_ADD;
    p_value->value = ip;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u32_t));
}

int spp_sip_del(int nl_fd, u32 ip)
{
    spp_comm_msg_t msg;
    spp_comm_u32_t *p_value = (spp_comm_u32_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_SIP_DEL;
    p_value->value = ip;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u32_t));
}


int spp_cip_add(int nl_fd, u32 ip)
{
    spp_comm_msg_t msg;
    spp_comm_u32_t *p_value = (spp_comm_u32_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_CIP_ADD;
    p_value->value = ip;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u32_t));
}

int spp_cip_del(int nl_fd, u32 ip)
{
    spp_comm_msg_t msg;
    spp_comm_u32_t *p_value = (spp_comm_u32_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_CIP_DEL;
    p_value->value = ip;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u32_t));
}

int spp_wmm_set(int nl_fd, bool state)
{
    spp_comm_msg_t msg;
    spp_comm_u16_t *p_value = (spp_comm_u16_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_WMM_SET;
    p_value->value = state;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u16_t));
}

int spp_dup_set(int nl_fd, u16 index,u16 val)
{
    spp_comm_msg_t msg;
    spp_comm_dup_set_t *p_value = (spp_comm_dup_set_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_DUP_SET;
    p_value->index = index;
    p_value->value = val;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_dup_set_t));
    
}

int spp_trim_set(int nl_fd, u16 v)
{
    spp_comm_msg_t msg;
    spp_comm_u16_t *p_value = (spp_comm_u16_t *)&msg.data[0];
    
    msg.msg_type = SPP_COMM_TRIM_SET;
    p_value->value = v;
    
    return spp_netlink_send_msg(nl_fd, (u8 *)&msg, sizeof(msg) + sizeof(spp_comm_u16_t));
}

