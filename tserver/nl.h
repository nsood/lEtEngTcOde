#ifndef NL_H_
#define NL_H_
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t u8;

#define SPP_NETLINK_PROTO 25

typedef enum{
    SPP_COMM_SIP_ADD        = 0,
    SPP_COMM_SIP_DEL,
    SPP_COMM_CIP_ADD,
    SPP_COMM_CIP_DEL,
    
    SPP_COMM_WMM_GET,
    SPP_COMM_WMM_GET_ACK,
    SPP_COMM_WMM_SET,
    
    SPP_COMM_DUP_GET,
    SPP_COMM_DUP_GET_ACK,
    SPP_COMM_DUP_SET,
    
    SPP_COMM_TRIM_GET,
    SPP_COMM_TRIM_GET_ACK,
    SPP_COMM_TRIM_SET,
}spp_comm_type_t;

typedef struct{
    u16 msg_type;
    u8  data[0];
}spp_comm_msg_t;

typedef struct{
    u16 value;
}spp_comm_u16_t;

typedef struct{
    u32 value;
}spp_comm_u32_t;

typedef struct{
    u16 index;
    u16 value;
}spp_comm_dup_set_t;


/*以下为对外接口*/

/*创建netlink socket*/
int spp_netlink_connect(int protocol);
/*关闭netlink socket*/
void spp_netlink_close(int nl_fd);

/*添加server/client ip*/
int spp_sip_add(int nl_fd, u32 ip);
int spp_sip_del(int nl_fd, u32 ip);
int spp_cip_add(int nl_fd, u32 ip);
int spp_cip_del(int nl_fd, u32 ip);

/*开启/关闭wmm特性*/
bool spp_wmm_get(int nl_fd);
int spp_wmm_set(int nl_fd, bool state);

/*设置/获取多发时间间隔*/
u16 spp_dup_get(int nl_fd, u16 index);
/*index 第几次 val为间隔值*/
int spp_dup_set(int nl_fd, u16 index,u16 val);

/*设置去重有效时间*/
u16 spp_trim_get(int nl_fd);
int spp_trim_set(int nl_fd, u16 v);

#endif//nl.h
