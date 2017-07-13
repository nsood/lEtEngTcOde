#include "include.h"
#include "tserver.h"
#include "tq_qos.h"
#include "tq_mach_api.h"

extern struct tq_ser_ctl ser_ctl;
extern struct tq_cfg_s cfg_ctl;

/***********************************************************
*name		:	acquire_ip_qos_list
*function	:	acquire all client ip,and strcat it's speed limit
*argument	:	void
*return		:	'x.x.x.x,x,x;x.x.x.x,x,x' special json type string
*notice		:	collect ip from udhcpc.leases,
				so it can't the get client configed by static way
***********************************************************/
int acquire_ip_qos_list(char *ip_qos_list,size_t size)
{
	size_t orig = size;
	char ipAddr[64];
	FILE *fp;
	struct in_addr addr;
	struct tq_ser_ctl *ctl = &ser_ctl;
	fp = fopen("/var/udhcpd.leases", "r");
	if (fp != NULL) {
		while (size  &&fread(&dhcp_lease, 1, sizeof(dhcp_lease),fp) == sizeof(dhcp_lease)) {
			addr.s_addr = dhcp_lease.ip;
			snprintf(ipAddr,sizeof(ipAddr),inet_ntoa(addr));
#if 1
			//cip pass...
			int i=0,flag=0;
			for(i=0;i<CLI_MAX;i++) {
				if(ctl->tq_qos.qos_cli[i].en==1 && 0==strcmp(ipAddr,ctl->tq_qos.qos_cli[i].ip)) {
					flag = 1;
				}
			}
			if(flag>0) {
				debug_info("CIP %s(qos passed)",ipAddr);
				continue;
			}
#endif
			debug_info("IP:%s",ipAddr);
			int brandlimit = (ctl->tq_qos.brandlimit>0)?ctl->tq_qos.brandlimit*1000:1000;
			size_t sn = snprintf(ip_qos_list,size,"%s,%d,%d;",inet_ntoa(addr),brandlimit,brandlimit);
			ip_qos_list += sn;
			size -= sn;
		}
		fclose(fp);
		*(--ip_qos_list) = '\0';//deal with last ';'
	}
	return (int)(orig - size);
}

/***********************************************************
*name		:	start_qos
*function	:	task of starting qos function,system call  nvram_set
*argument	:	void
*return		:	void
*notice		:	cause write nvram flash waste a little time,
				so there need sleep after the call at backgound
***********************************************************/
void start_qos()
{
	char buf[SIZE_BASE]={'\0'};
	char ip_qos_list[SIZE_BASE]={'\0'};
	struct tq_ser_ctl *ctl = &ser_ctl;

	nvram_set("QoSEnable", "2");
	int ips_len = acquire_ip_qos_list(ip_qos_list,sizeof(ip_qos_list));
	if(0 == ips_len || ips_len >= sizeof(ip_qos_list)) {
		debug_info("get ip list fail!");
		return;
	}
	strncpy(ctl->tq_qos.ip_list,ip_qos_list,ips_len);
	sprintf(buf,"\"%s\"",ip_qos_list);

	debug_info("\tip_qos_list : %s ",buf);
	debug_info("\ttimeout : %d",ctl->tq_qos.qos_tm_max);
	debug_info("\tcli_cnt : %d",ctl->tq_qos.cli_cnt);

	nvram_set("IpQosList", buf);
	sleep(4);
	system("jcc_ctrl updatenvram 0");
	system("jcc_ctrl restartqos &");
}

/***********************************************************
*name		:	record_new_client
*function	:	support mult-client of qos used
*argument	:	char *cip	:	client ip
				int times	:	this client dhcp_lease times
*return		:	void
*notice		:	if array have already exist cip, should update it's times
***********************************************************/
void record_new_client(char *cip,int times)
{
	int i=0;
	struct tq_ser_ctl *ctl = &ser_ctl;

	for(i = 0; i < CLI_MAX; i++) {
		if(0==strcmp(cip,ctl->tq_qos.qos_cli[i].ip)) {
			ctl->tq_qos.qos_cli[i].times = times;
		}
	}

	i=0;
	while( i<CLI_MAX
		&& 1==ctl->tq_qos.qos_cli[i].en
		&& 0!=strcmp(cip,ctl->tq_qos.qos_cli[i].ip))i++;
	if( i<CLI_MAX ) {
		debug_info("add ctl->tq_qos.qos_cli :[%d] %s",i,cip);
		ctl->tq_qos.qos_cli[i].en = 1;
		ctl->tq_qos.qos_cli[i].times = times;
		strcpy(ctl->tq_qos.qos_cli[i].ip,cip);
	}
}

/***********************************************************
*name		:	update_client_qos_info
*function	:	update every client qos info,include en,times,
				and calculate the count of enable client
				and compare and return the max times
*argument	:	void
*return		:	void
*notice		:	call at each loop of qos timer
***********************************************************/
void update_qos_info()
{
	int c=0,m=0,i=0;
	struct tq_ser_ctl *ctl = &ser_ctl;

	for ( i = 0; i < CLI_MAX; i++){
		if(ctl->tq_qos.qos_cli[i].en==1){
			debug_info("qos_clinet[%d] enable ip :%s",i,ctl->tq_qos.qos_cli[i].ip);
			ctl->tq_qos.qos_cli[i].times -= 1;
			if(ctl->tq_qos.qos_cli[i].times<=0){
				ctl->tq_qos.qos_cli[i].times=0;
				ctl->tq_qos.qos_cli[i].en=0;
			} else {
				c++;
			}
			if(ctl->tq_qos.qos_cli[i].times > m) {
				m = ctl->tq_qos.qos_cli[i].times;
			}
		}
	}
	ctl->tq_qos.qos_tm_max = m;
	ctl->tq_qos.cli_cnt = c>0 ? c : 0;
}

/***********************************************************
*name		:	qos_tm_handler
*function	:	qos loop timer handler,
				update and set ip_qos_list to nvram
				stop timer while some condition satisfied
				or continue loop qos timer
*argument	:	void
*return		:	void
*notice		:	
***********************************************************/
void qos_tm_handler()
{
	struct tq_ser_ctl *ctl = &ser_ctl;
	struct tq_cfg_s *cfg = &cfg_ctl;
	char buf[SIZE_BASE]={'\0'};
	char ip_qos_list[SIZE_BASE]={'\0'};

	//update currently clinets info
	update_qos_info();
	debug_info("timeout : %d",ctl->tq_qos.qos_tm_max);
	debug_info("cli_cnt : %d",ctl->tq_qos.cli_cnt);
	//refresh ip_list
	int ips_len = acquire_ip_qos_list(ip_qos_list,sizeof(ip_qos_list));
	if(0 == ips_len || ips_len >= sizeof(ip_qos_list)) {
		debug_info("get ip list fail!");
		return;
	}
	if (0 != strncmp(ctl->tq_qos.ip_list,ip_qos_list,ips_len)) {
		debug_info("refresh ip_qos_list");
		snprintf(buf,sizeof(buf),"\"%s\"",ip_qos_list);
		//debug_info("qos timeout ip_qos_list : %s ",buf);
		nvram_set("IpQosList", buf);
		sleep(4);
		system("jcc_ctrl updatenvram 0");
		system("jcc_ctrl restartqos &");
	}

	if(ctl->tq_qos.cli_cnt<=0 || ctl->tq_qos.qos_tm_max<=0 || ctl->tq_qos.enable==0) {
		ctl->tq_qos.qos_tm_max = 0;
		ctl->tq_qos.enable = 0;
		ctl->tq_qos.cli_cnt = 0;

		if (cfg->qos.en == 1) {
			nvram_set("QoSEnable", "2");
		} else {
			nvram_set("QoSEnable", "0");
		}

		sprintf(buf,"\"%s\"",cfg->qos.ip_list);
		//debug_info("qos timeout iplist : %s ",buf);
		nvram_set("IpQosList", buf);
		sleep(4);
		system("jcc_ctrl updatenvram 0");
		system("jcc_ctrl restartqos &");
		return;
	}

	if (ctl->tq_qos.enable>0 && ctl->tq_qos.cli_cnt>0){
		strncpy(ctl->tq_qos.ip_list,ip_qos_list,ips_len);
		ctl->tq_qos.tm = tq_timer_new(NULL, qos_tm_handler, TIME_QOS, "QOS loop Timer");
	}
}


