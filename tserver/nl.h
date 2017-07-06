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


/*����Ϊ����ӿ�*/

/*����netlink socket*/
int spp_netlink_connect(int protocol);
/*�ر�netlink socket*/
void spp_netlink_close(int nl_fd);

/*���server/client ip*/
int spp_sip_add(int nl_fd, u32 ip);
int spp_sip_del(int nl_fd, u32 ip);
int spp_cip_add(int nl_fd, u32 ip);
int spp_cip_del(int nl_fd, u32 ip);

/*����/�ر�wmm����*/
bool spp_wmm_get(int nl_fd);
int spp_wmm_set(int nl_fd, bool state);

/*����/��ȡ�෢ʱ����*/
u16 spp_dup_get(int nl_fd, u16 index);
/*index �ڼ��� valΪ���ֵ*/
int spp_dup_set(int nl_fd, u16 index,u16 val);

/*����ȥ����Чʱ��*/
u16 spp_trim_get(int nl_fd);
int spp_trim_set(int nl_fd, u16 v);

#endif//nl.h
