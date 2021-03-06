#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "include.h"
#include "tq_json.h"
#include "tserver.h"
#include "tq_mach_api.h"
#include "debug.h"

int tq_daemon()
{
	int fd;
	switch(fork())
	{
		case -1:
			return -1;
			break;
		case 0:
			break;
		default:
			exit(0);
	}
	int ret = setsid();
	if(ret == -1)
	{
		perror("setsid");
		return -1;
	}
	fd = open("/dev/null", O_RDWR);
	if(fd == -1)
	{
		perror("open");
		return -1;
	}
	ret = dup2(fd, STDIN_FILENO);
	if(ret == -1)
	{
		perror("dup2");
		return -1;
	}
	ret = dup2(fd, STDOUT_FILENO);
	if(ret == -1)
	{
		perror("dup2");
		return -1;
	}
	if(fd > STDERR_FILENO)
	{
		close(fd);
	}
	return 0;
}

void reset_qos_config_from_file()
{
	debug_info("Reset qos config cause an accident.");
	struct json_object *qos_cfg_file_js = NULL;
	qos_cfg_file_js = json_object_from_file("/data/qos_cfg.file");
	if (NULL != qos_cfg_file_js) {
		char qos_cfg_buf[SIZE_BASE]={'\0'};
		sprintf(qos_cfg_buf,"/tmp/qos.sh \"%s\" \"%s\" &",
			tq_json_get_string(qos_cfg_file_js,"QoSEnable"),
			tq_json_get_string(qos_cfg_file_js,"IpQosList"));
		system(qos_cfg_buf);
		debug_info("Reset : %s",qos_cfg_buf);
		json_object_put(qos_cfg_file_js);
	}
}
#if 1
void sighandler(int sig,siginfo_t *si,void *ctx)
{
	if(sig == SIGSEGV)
	{
		debug_info("SIGSEGV:%d %p\n", sig,ctx);
	}

	//deal qos accidence
	system("rmmod spp");
#if 0
	set_nvram("QoSEnable", "0");
	sleep(4);
	system("jcc_ctrl updatenvram 0");
	system("jcc_ctrl restartqos &");
#else
	if(0 == access("/data/qos_cfg.file", F_OK)) {
		reset_qos_config_from_file();
	}
#endif
	int i = 0;
	//close(epfd);
	for(i = 0; i < 3; i++)
	{
		close(fdis[i]->fd);
		free(fdis[i]);
		fdis[i] = NULL;
	}
	free_all_timer();

	system("[ `ps -ef | sed -n '/ltm/p' | wc -l` -gt 1 ] && killall ltm && echo 'accident raised : kill ltm by tqos'>/dev/console");

}
#else
void sighandler(int sig)
{
	if(sig == SIGSEGV)
	{
		debug_info("SIGSEGV:%d\n", sig);
	}
	int i = 0;
	//close(epfd);
	for(i = 0; i < 3; i++)
	{
		close(fdis[i]->fd);
		free(fdis[i]);
		fdis[i] = NULL;
	}
	free_all_timer();
}
#endif
time_t getDateFromMacro(char const *time) {
    char s_month[5];
    int month, day, year;
    struct tm t = {0};
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(time, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;

    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}

bool check_copyright_time()
{
	time_t time_build = getDateFromMacro(__DATE__);
	struct tm *p = localtime(&time_build);

	p->tm_year += 1;

	time_t time_after_build = mktime(p);
	time_t time_run_now = time(NULL);
	if (difftime(time_after_build,time_run_now) > 0)
		return true;
	else
		return false;
}
int main(int argc, char *argv[])
{

    int c;

	system("[ -f /tmp/debug.file ] && rm /tmp/debug.file");
	debug_info("init log...");
	//print build time
	debug_info(">> TQOS Build");
	debug_info(">> Date : %s",__DATE__);
	debug_info(">> Time : %s",__TIME__);
	//set a time to control software time limit
	if(!check_copyright_time()) exit(1);

	system("[ -f /tmp/qos.sh ] && rm /tmp/qos.sh");
	system("echo '#!/bin/sh' >> /tmp/qos.sh");
	system("echo 'nvram_set 2860 QoSEnable $1' >> /tmp/qos.sh");
	system("echo 'if [ ! -z $2 ];then' >> /tmp/qos.sh");
	system("echo 'nvram_set 2860 IpQosList $2' >> /tmp/qos.sh");
	system("echo 'else' >> /tmp/qos.sh");
	system("echo 'nvram_set 2860 IpQosList \"\"' >> /tmp/qos.sh");
	system("echo 'fi' >> /tmp/qos.sh");
	system("echo 'jcc_ctrl updatenvram 0' >> /tmp/qos.sh");
	system("echo 'jcc_ctrl restartqos' >> /tmp/qos.sh");
	system("chmod a+x /tmp/qos.sh");
	debug_info("init qos script...\n");

	if(0 == access("/data/qos_cfg.file", F_OK)) {
		reset_qos_config_from_file();
		system("[ -f /data/qos_cfg.file ] && rm /data/qos_cfg.file && echo 'rm qos_cfg.file success.'");
	}
    while(1)
    {
        int option_index = 0;
        static struct option long_options[] = 
        {
	        {"daemon", no_argument,		NULL, 'd'},
            {0,      0,                	0,  0 }
        };
        c = getopt_long(argc, argv, "d", long_options, &option_index);
        if(c == -1)
            break;

        switch(c)
        {
            case 'd':
                tq_daemon();
                break;
            default:
                break;
        }
    }
    if(optind < argc)
    {
        printf("non-option ARGV-elements:");

		while(optind < argc)
        {
            debug_info("%s ", argv[optind++]);
        }
        return 1;

    }

	struct sigaction act;
#if 0
	act.sa_handler = sighandler;
#else
	act.sa_sigaction = sighandler;
#endif
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	timer_init();
	//add_timer_test();
    tq_server_start();
	debug_info("server clean up...");
}
