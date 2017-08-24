#include <stdio.h>
#include <string.h>

#define ROUTER_BOARD 		"MT7620"
#define PLUGIN_TIME 		"/tmp/tqos.time"
#define PLUGIN_MD5PATH		"/tmp/tqos.md5"
#define PLUGIN_VENDOR		"jcg-tqos"
#define PLUGIN_IFNAME		"br0"
#define PLUGIN_URL			"/backend/check.php"
#define PLUGIN_INTERVAL		21600


static const char *compile_time = __DATE__"-"__TIME__;

static char *def_servers[] = {
	 "39.108.167.249",
	 "www.zyrui.com",
	 NULL,
};

static struct lt_element{
	char *vendor;
	char *ifname;
	char *md5path;
	char **servers;
	char *url;
	int interval;
} element = {
	PLUGIN_VENDOR,
	PLUGIN_IFNAME,
	PLUGIN_MD5PATH,
	def_servers,
	PLUGIN_URL,
	PLUGIN_INTERVAL,
};

void element_record_url(char *url)
{
	element.url = url;
}
void element_record_md5(char *m)
{
	element.md5path = m;
}
void element_record_cycle(int i)
{
	element.interval = i;
}
void element_record_iface(char *ifname)
{
	element.ifname = ifname;
}
void element_record_server(char **s)
{
	element.servers = s;
}
void element_record_vendor(char *c)
{
	element.vendor = c;
}
void element_record_plugintime(void)
{
	FILE *fp = fopen(PLUGIN_TIME,"w+");
	if (!fp)
		return ;
	fwrite(compile_time,strlen(compile_time),1,fp);
	fclose(fp);
}


const char *element_board(void)
{
	return ROUTER_BOARD;
}
const char *element_md5path(void)
{
	return element.md5path;
}
const char *element_iface(void)
{
	return element.ifname;
}
const char *element_url(void)
{
	return element.url;
}
char **element_servers(void)
{
	return element.servers ;
}
int element_cycle(void)
{
	return element.interval;
}
const char *element_vendor(void)
{
	return element.vendor;
}

