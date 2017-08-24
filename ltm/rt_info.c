#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/socket.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "ioswitch.h"
#include "rt_info.h"
#include "element.h"


bool hardware_get_sysinfo(struct router_info *ri,const char *iface)
{
	bool res = false;
	FILE *fp  = NULL;
	
	struct utsname ubuf;
	if (uname(&ubuf))
		return res;
	
	strncpy(ri->machine,ubuf.machine,sizeof(ri->machine));
	strncpy(ri->version,ubuf.release,sizeof(ri->version));
	strncpy(ri->plug_ver,element_board(),sizeof(ri->plug_ver));
	strncpy(ri->customer_id,element_vendor(),sizeof(ri->customer_id));
	
	struct ifreq ifr;
	memset(&ifr,0,sizeof(ifr));
	/*fix eth0*/
	strcpy(ifr.ifr_name,iface);
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
		return res;

	if (0 > ioctl(sock,SIOCGIFHWADDR,&ifr)) {
		gdb_lt("get %s's hwaddr error",iface);
	} else {
		snprintf(ri->mac,sizeof(ri->mac),"%02x%02x%02x%02x%02x%02x",
						(unsigned char)ifr.ifr_hwaddr.sa_data[0],
						(unsigned char)ifr.ifr_hwaddr.sa_data[1],
						(unsigned char)ifr.ifr_hwaddr.sa_data[2],
						(unsigned char)ifr.ifr_hwaddr.sa_data[3],
						(unsigned char)ifr.ifr_hwaddr.sa_data[4],
						(unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	}
	/*bug??*/
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_name,iface);
	
	if (0 > ioctl(sock,SIOCGIFADDR,&ifr)) {
		gdb_lt("get %s's ipaddr error",iface);
	} else {
		struct sockaddr_in *psin = (struct sockaddr_in *)&ifr.ifr_addr;
		inet_ntop(AF_INET,&psin->sin_addr,ri->ip,sizeof(ri->ip));
	}
	
	fp = fopen("/proc/cpuinfo","r");
	if (!fp)
		goto clean;
	
	char buf[1024];
	int nr = fread(buf,1,sizeof(buf) - 1,fp);
	if (nr < 0)
		goto clean;
	buf[nr] = '\0';

	char *cpu = strstr(buf,"system type");
	if (!cpu)
		cpu = strstr(buf,"model name");

	if (!cpu)
		goto clean;

	while (*cpu && *cpu != ':')
		cpu++;
	if (!(*cpu++))
		goto clean;
	
	while (isspace(*cpu))
		cpu++;

	int i;
	for (i = 0 ; *cpu && *cpu != '\n' && i < sizeof(ri->system) - 1 ; i++)
		ri->system[i] = *cpu++;
	ri->system[i] = '\0';

	res = true;
clean:
	if (fp) 
		fclose(fp);
	close(sock);
	return res;
}


