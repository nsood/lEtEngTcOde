#ifndef _SPP_COMM_H
#define _SPP_COMM_H

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
    u16 index;
    u16 value;
}spp_comm_dump_set_t;

int spp_comm_init(void);
void spp_comm_fini(void);


#endif

