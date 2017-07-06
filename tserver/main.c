#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include "include.h"
#include "tserver.h"

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
#if 1
void sighandler(int sig,siginfo_t *si,void *ctx)
{
	if(sig == SIGSEGV)
	{
		debug_info("SIGSEGV:%d %p\n", sig,ctx);
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
int main(int argc, char *argv[])
{

    int c;
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


	system("[ -f /tmp/debug.file ] && rm /tmp/debug.file");
	debug_info("init log\n");

	struct sigaction act;
#if 0
	act.sa_handler = sighandler;
#else
	act.sa_sigaction = sighandler;
#endif
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGKILL, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	timer_init();
	//add_timer_test();
    tq_server_start();
	debug_info("server clean up...");
}
