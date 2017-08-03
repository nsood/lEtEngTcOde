#include "include.h"
#include "tserver.h"
#include "tq_mach_api.h"

/***********************************************************
*name		:	sys_popen
*function	:	system call popen package
*argument	:	const char *cmd		:	command line would be runned
				char *result			:	point of result string
*return		:	point of result string
*notice		:	deal last char '\n'
***********************************************************/
char *sys_popen(const char *cmd, char *result, int m_len)
{	
	char tmp[SIZE_BASE]={'\0'};
	FILE *fp;
	//debug_info("sys_popen cmd : %s",cmd);

	memset(result,0,m_len);
	if (NULL!=(fp = popen(cmd,"r"))) {
		while(NULL!=fgets(tmp,SIZE_BASE,fp)){
			strcat(result,tmp);
		}
		pclose(fp);
		result[strlen(result)-1] = '\0';//deal '\n'
	} else {
		strcpy(result,"");
	}
	//debug_info("sys_popen result : %s",result);
	return result;
}

/***********************************************************
*name		:	get_nvram
*function	:	acquire key-value store in nvram 2860
*argument	:	const char *name	:	key string
*return		:	system call return point
*notice		:	
***********************************************************/
char *get_nvram(const char *name, char *nvram_buf)
{
	char buf[SIZE_NVRAM];
	sprintf(buf,"nvram_get 2860 %s",name);
	debug_info("\t%s",buf);
	return sys_popen(buf,nvram_buf,SIZE_NVRAM);
}

/***********************************************************
*name		:	set_nvram
*function	:	set key-value
*argument	:	const char *name	:	key string
				const char *args		:	value string
*return		:	void
*notice		:	
***********************************************************/
void set_nvram(const char *name, const char *args)
{
	char buf[SIZE_NVRAM];
	memset(buf,0,SIZE_NVRAM);
	snprintf(buf,sizeof(buf),"nvram_set 2860 %s %s &",name,args);
	debug_info("\t%s",buf);
	system(buf);
}

int GetWlanList(char *wlanif, void *table)
{
	int s, cmd;
	struct iwreq iwr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, wlanif, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t)table;

	if (s < 0) {
		printf("ioctl sock failed!\n");
		return -1;
	}

#if 1//def CONFIG_RT2860V2_AP_V24_DATA_STRUCTURE
	cmd = RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT;
#else
	cmd = RTPRIV_IOCTL_GET_MAC_TABLE;
#endif

	if (ioctl(s, cmd, &iwr) < 0) {
		printf("%s[%d] ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE failed!\n", __FILE__, __LINE__);
		close(s);
		return -1;
	}
	close(s);
	return 0;
}

int acquire_rssi(const char *mac)
{
	int rssi_i=0;
	RT_802_11_MAC_TABLE table = {0};

	GetWlanList("ra0", &table);
	//debug_ser("ret: %d\n", ret);
	if(table.Num > 0) {

/*		int i=0;
		for(i=0; i<table.Num; i++) {
			RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
			debug_ser("RSSI: %d:%d:%d", pe->AvgRssi0, pe->AvgRssi1, pe->AvgRssi2);
			rssi =(pe->AvgRssi0 > pe->AvgRssi1) ? pe->AvgRssi0 : pe->AvgRssi1;
		}
*/
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[0]);
		debug_info("RSSI: %d:%d:%d", pe->AvgRssi0, pe->AvgRssi1, pe->AvgRssi2);
		rssi_i =(pe->AvgRssi0 > pe->AvgRssi1) ? pe->AvgRssi0 : pe->AvgRssi1;
		debug_info("rssi : %d",rssi_i);
	}
	return rssi_i;
}

int acquire_losepack()
{
	return 0;
}
int acquire_errpack()
{
	return 0;
}

int acquire_terminals()
{
//	return 0;
	int count=0;
	char buf[10]={'\0'};
	sys_popen("cat /proc/net/arp | sed -n '/[0-9]\\{1,3\\}.[0-9]\\{1,3\\}.[0-9]\\{1,3\\}.[0-9]\\{1,3\\}/p' | sed -n '$='", buf, sizeof(buf));
	count = atoi(buf);
	debug_info("terminals:%d", count);
	return count;
}
int acquire_channelrate()
{
	return 0;
}

int acquire_errorrate()
{
	return 0;
}

int acquire_memory()
{
	int total,free;
	char cpu_mem_buf[SIZE_CPU_MEM]={'\0'};
	sys_popen("cat /proc/meminfo | sed -n '/MemTotal/p' | sed 's/[^0-9]//g'",cpu_mem_buf,sizeof(cpu_mem_buf));
	if (NULL!=cpu_mem_buf) {
		total = atoi(cpu_mem_buf);
	} else {
		return 0;
	}
	sys_popen("cat /proc/meminfo | sed -n '/MemFree/p' | sed 's/[^0-9]//g'",cpu_mem_buf,sizeof(cpu_mem_buf));
	if (NULL!=cpu_mem_buf) {
		free = atoi(cpu_mem_buf);
	} else {
		return 0;
	}
	debug_info("MEM:%d%%",(100-100*free/total));
	return (100-100*free/total);
}

int acquire_cpu()
{
	int cpu;
	char avg[SIZE_CPU_MEM];
	char cpu_mem_buf[SIZE_CPU_MEM]={'\0'};
#ifdef OPENWRT
	strcpy(avg,sys_popen("top -n 1 | sed -n '/^CPU/p' | sed -n 's/CPU: *//p' | sed -n 's/% usr.*//p'",cpu_mem_buf,sizeof(cpu_mem_buf)));
#else
	strcpy(avg,sys_popen("cpu | sed -n '/[0-9]/p'",cpu_mem_buf,sizeof(cpu_mem_buf)));
#endif
	if (NULL!=cpu_mem_buf) {
		cpu = atoi(strtok(avg," "));
		debug_info("\tCPU:%d %%",cpu);
		return cpu;
	} else {
		return 0;
	}
}


/***********************************************************
*name		:	tq_acquire_mac
*function	:	acquire mobile client MAC
*argument	:	char *ip		:	client ipv4
*return		:	MAC string point
*notice		:	search mac from udhcpc.leases,
				so it can't the get client configed by static way
***********************************************************/
char *tq_acquire_mac(char *ip, char *mac_buf)
{
	char macAddr[32], ipAddr[32], ip_s[32];
	FILE *fp;
	struct in_addr addr;

	memcpy(ip_s,ip,sizeof(ip_s));
	fp = fopen("/var/udhcpd.leases", "r");
	if (fp != NULL) {
		debug_info("Client ip : %s",ip_s);
		while (fread(&dhcp_lease, 1, sizeof(dhcp_lease),fp) == sizeof(dhcp_lease)) {
			snprintf(macAddr,sizeof(macAddr), "%02X:%02X:%02X:%02X:%02X:%02X",
				dhcp_lease.mac[0], dhcp_lease.mac[1], dhcp_lease.mac[2],
				dhcp_lease.mac[3], dhcp_lease.mac[4], dhcp_lease.mac[5]);
			addr.s_addr = dhcp_lease.ip;
			strcpy(ipAddr, inet_ntoa(addr));
			debug_info("Host: %s, MAC: %s, IP: %s", dhcp_lease.hostname,macAddr, ipAddr);
			if(0==strcmp(ip_s,ipAddr)) {
				memset(mac_buf,0,SIZE_MAC);
				strcpy(mac_buf,macAddr);
				fclose(fp);
				return mac_buf;
			}
		}
		fclose(fp);
	}
	return NULL;
}

/***********************************************************
*name		:	tq_acquire_ver
*function	:	acquire route VER though file inside
*argument	:	char *v		:	stored point
*return		:	void
*notice		:	
***********************************************************/
char *tq_acquire_ver(char *v)
{
	char p[20];
	sys_popen("cat /etc_ro/versions.txt | sed -n '/versionDisp/p' | sed -n 's/.*=//p'", p,sizeof(p));

	strcpy(v,"JCGV");
	strcat(v,p);	
	return v;
}
