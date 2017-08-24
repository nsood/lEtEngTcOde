#include <unistd.h>
#include <getopt.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "ioswitch.h"
#include "update.h"
#include "rt_info.h"
#include "various.h"
#include "element.h"
#include "wgetfile.h"


static const struct option largs[] = {
		{"interface",required_argument,NULL,'i'},
		{"log-on",no_argument,NULL,'o'},
		{"router",required_argument,NULL,'r'},
		{"remote-server",required_argument,NULL,'s'},
		{"remote-url",required_argument,NULL,'u'},
		{"plugin-md5path",required_argument,NULL,'f'},
		{"time",required_argument,NULL,'t'},
		{"compile",no_argument,NULL,'b'},
		{NULL,0,NULL,0},
}; 

static char *servers[2] = {NULL,NULL};

static void sig_hand(int sig)
{
	gdb_lt("get sig %d",sig);
	system("[ -f /tmp/tqos.md5 ] && rm /tmp/tqos.md5");
	system("[ `ps -ef | sed -n '/tqos/p' | wc -l` -gt 1 ] && killall tqos && echo 'accident raised : kill tqos by ltm' >/dev/console");
	exit(0);
}

int main(int argc,char **argv)
{
	background();

	int sec = 0;
	int value = -1,index = 0;
	
	while ((value = getopt_long(argc,argv,"i:t:s:u:r:f:ob",largs,&index)) != -1) {
		switch (value) {
		case 'o':	
			ioctl_switch();
		break;
		case 'i':
			element_record_iface(optarg);
		break;
		case 'r':
			element_record_vendor(optarg);
		break;
		case 'f':
			element_record_md5(optarg);
		break;
		case 't':
			sec = atoi(optarg);
			if (sec > 0)
				element_record_cycle(sec);
		break;
		case 's':
			servers[0]=optarg;
			element_record_server(servers);
		break;
		case 'u':
			element_record_url(optarg);
		break;
		case 'b':
			element_record_plugintime();
			return 0;
		break;
		}
	}

	signal(SIGTERM,sig_hand);
	signal(SIGKILL,sig_hand);
	signal(SIGINT,sig_hand);
	signal(SIGSEGV,sig_hand);
		
	engine_start();
	return 0;
}
