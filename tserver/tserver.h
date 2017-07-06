#ifndef TSERVER_H_
#define TSERVER_H_

#include "include.h"

#define RTDEV_SUPPORT

#define INT_INV (0x7FFFFFF)

#define SIZE_IP 32
#define SIZE_MAC 32
#define SIZE_NVRAM 1024
#define SIZE_CPU_MEM 128
#define SIZE_BASE 1024
#define SEC_60 60
#define SEC_5 5
#define TIME_QOS	20
#define DEF_EXPIRE 7200
#define SIZE_DIAG 12
#define CLI_MAX 64
#define SIZE_BUF_ARRAY SIZE_BASE*2
#define SIZE_BUF_DEALT SIZE_BASE*2
#define SIZE_BUF_TRAC SIZE_BASE*3

#define SIZE_VER 128
#define PATH_VER "/etc_ro/versions.txt"

#define PORT_V		17777
#define PORT_O		9999
#define SIZE_BUF_MAX	SIZE_BASE*2
#define SIZE_BUF_REQ 	(SIZE_BUF_MAX - sizeof(struct request_s))
#define SIZE_BUF_MSG 	(SIZE_BUF_MAX -sizeof(struct request_s) + sizeof(struct req_msg))
#define SIZE_BUF_CTT (SIZE_BUF_MAX - sizeof(struct request_s))
#define SIZE_ROUND		8
#define EVENT_MAX		8

#define MIN_TYPE	1000
#define GET_VER		1001
#define REQ_AUTH	1003
#define OPEN_QOS	1005
#define OPEN_SPP	1007
#define END_SPEED	1009
#define OPEN_INFO	1011
#define END_INFO	1013
#define DIAG_AUTH	1015

// struct for udhcpd.leases
struct dhcp_lease_s {
	unsigned char hostname[16];
	unsigned char mac[16];
	unsigned long ip;
	unsigned long expires;
#if 1//def CONFIG_JCG_H1PAGE_SUPPORT
	unsigned char vendor[16];
#endif
} dhcp_lease;

struct req_msg{
    uint32_t m_bId;
    uint16_t m_ver;
    uint16_t m_type;
    uint16_t m_cont_len;
    char m_cont[0];
};

struct request_s{
	int fd;
    struct sockaddr_in cli_addr;
    struct req_msg msg;
};

struct qos_cli{
	int en;
	char ip[SIZE_IP];
	int times;
};

struct tq_qos_ctl{
	u8 enable;
	struct qos_cli qos_cli[CLI_MAX];
	int qos_tm_max;
	int cli_cnt;
	rbtree_node_t tm;
	char ip_list[SIZE_BASE];
};

struct tq_diag_data{
	long time;
	int errpack;
	int losepack;
	int cpu;
	int memory;
	char traffic[SIZE_BUF_TRAC];
};

struct tq_diag_ctl{
	int enable;
	int last_index;
	rbtree_node_t tm;
	struct tq_diag_data diagnoses[SIZE_DIAG];
};

struct tq_ser_ctl{
	struct tq_qos_ctl tq_qos;
	struct tq_diag_ctl tq_diagnose;
};

struct spp_cfg{
	int doublegap;
	int thirdgap;
	int fourgap;
	int dupgap;
};

struct qos_cfg {
	int en;
	char ip_list[SIZE_BASE];
};

struct tq_cfg_s {
	char	interface[10];
	u32		ips[CLI_MAX];
	u32		ipd[CLI_MAX];
	struct 	spp_cfg gap;
	struct	qos_cfg qos;
};

extern struct fdinfo *fdis[3];
extern int epfd;
struct fdinfo
{
    int flag;
    int fd;
};

void tq_server_start(void);

#endif//tserver.h
