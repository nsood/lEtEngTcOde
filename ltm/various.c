#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "types.h"
#include "ioswitch.h"

bool name2addr(const char *s,struct sockaddr_in *sa)
{
	struct addrinfo *ai = NULL;
	int ret,tries = 10;
	while (tries-- > 0) {
		ret = getaddrinfo(s,NULL,NULL,&ai);
		if (ret != EAI_AGAIN)
			break;
		sleep(3);
	}

	if (ret != 0)
		return false;
	
	memcpy(sa,ai->ai_addr,sizeof(*sa));
	freeaddrinfo(ai);
	return true;
}

char *split_line(char *s, char *e,int sep,int end,cstr_t *elts,uint n,uint *res)
{
	uint i;
	bool space;
	
	for (i = 0,space = true ; s < e  && i <= n ; s++) {
		if (*s == end) 
			break;

		if (*s == sep) {
			space = true;
			continue;
		}
		
		if (space) {
			i++;
			space = false;
			elts[i-1].str = s;
			elts[i-1].len = 0;
		} 
		
		elts[i - 1].len++;		
	}

	if (res) 
		*res = i ;
	return s + 1;
}

#if 0
void background(void)
{
	pid_t pid = fork();

	switch (pid) {
	case 0:
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	break;
	case -1:
		gdb_lt("fork error");
	break;
	default:
		exit(-1);
	}
	
}
#else
void background(void)
{
	int fd;
	switch(fork())
	{
		case -1:
			gdb_lt("fork error");
			break;
		case 0:
			break;
		default:
			exit(-1);
	}
	int ret = setsid();
	if(ret == -1)
	{
		gdb_lt("setsid");
		return ;
	}
	fd = open("/dev/null", O_RDWR);
	if(fd == -1)
	{
		gdb_lt("open");
		return ;
	}
	ret = dup2(fd, STDIN_FILENO);
	if(ret == -1)
	{
		gdb_lt("dup2");
		return ;
	}
	ret = dup2(fd, STDOUT_FILENO);
	if(ret == -1)
	{
		gdb_lt("dup2");
		return ;
	}
	if(fd > STDERR_FILENO)
	{
		close(fd);
	}
	return ;
}
#endif

bool ha_strcmp(cstr_t *cs,char *s,uint len)
{
	if (cs->len != len)
		return false;
	return (0 == strncasecmp(cs->str,s,len));
}

