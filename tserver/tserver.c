#include "tserver.h"
#include "tq_aes.h"
#include "tq_qos.h"
#include "tq_diag.h"
#include "tq_req_resp.h"

int epfd;
struct fdinfo *fdis[3] = {0};

struct tq_ser_ctl ser_ctl;
struct tq_ctt_s ctt_s;
struct tq_cfg_s cfg_ctl;

//static int round_index=0;
//static char g_req[SIZE_ROUND][SIZE_BUF_MAX];
static unsigned char g_req[SIZE_BUF_MAX];
int tq_ser_recv_req(int fd)
{
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	int n;
//    struct request_s *req = (struct request_s*)g_req[round_index];
//	debug_info("recv msg use g_req[%d]",round_index);
//	round_index = ( round_index + 1 ) % SIZE_ROUND;
	struct request_s *req = (struct request_s*)g_req;

	memset(req,0,SIZE_BUF_MAX);
    req->fd = fd;
	n = recvfrom(fd, (void*)&req->msg, SIZE_BUF_MSG - 1, 0, (struct sockaddr *)&req->cli_addr, &len);
	if(n == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
    	{
        	//
        	return 1;
    	}
    	else
    	{
			perror("recvfrom");
			return -1;
    	}
	}
	else if (n ==0)
	{
		return 0;
	}
	else
	{
		int orignal = ntohs(req->msg.m_cont_len) + sizeof(struct req_msg);
		debug_info("receive orignal : %d ",orignal);
		if(orignal-n <= 2)
		{
			debug_info("cli_ip:%s",inet_ntoa(req->cli_addr.sin_addr));
			debug_info("msg_type:%d",ntohs(req->msg.m_type));
			debug_info("msg_cont_len:%d",ntohs(req->msg.m_cont_len));
			tq_request_parse(req);
			return 0;
		}
		else
		{
			debug_info("Incomplete data, recv length:%d,orignal length:%d",n,orignal);
			return -1;
		}
	}
}


void set_nonblocking(int fd)
{
	int status_flag = 0;
	status_flag = fcntl(fd, F_GETFL);
	if(status_flag == -1)
	{
		perror("fcntl");
		exit(1);
	}
	status_flag |= O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, status_flag);
	if(ret == -1)
	{
		perror("fcntl");
		exit(1);
	}
}

static void server_init(void)
{
    struct fdinfo *fi = NULL;
    struct sockaddr_in srvaddr_v, srvaddr_o;
    int fds[3] = {0};
    struct epoll_event ev;
	struct tq_ser_ctl *ctl = &ser_ctl;
	struct tq_cfg_s *cfg = &cfg_ctl;

	debug_info("server init...");
	//init cfg
	memset(cfg,0,sizeof(struct tq_cfg_s));
	strcpy(cfg->interface,"br0");
	//init ctl
	memset(ctl,0,sizeof(struct tq_ser_ctl));
	ctl->tq_diagnose.enable = 0;
	ctl->tq_qos.enable = 0;
	ctl->tq_qos.brandlimit = 0;
	ctl->tq_qos.cli_cnt = 0;
	ctl->tq_qos.qos_tm_max = 0;
	//system("insmod spp.ko lan=\"br0\" wan=\"eth2.2\"");
	system("insmod spp.ko lan=\"br0\"");
    
	//create udp server fd for get VER info
    fds[0] = socket(AF_INET, SOCK_DGRAM, 0);
    if(fds[0] == -1)
    {
        perror("socket");
        exit(1);
    }
    set_nonblocking(fds[0]);
    memset(&srvaddr_v, 0, sizeof(srvaddr_v));
    srvaddr_v.sin_family = AF_INET;
    srvaddr_v.sin_port = htons(PORT_V);
    srvaddr_v.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(fds[0], (struct sockaddr *)&srvaddr_v, sizeof(srvaddr_v));

    //create udp server fd for getting CMD
    fds[1] = socket(AF_INET, SOCK_DGRAM, 0);
    if(fds[1] == -1)
    {
        perror("socket");
        exit(1);
    }
    set_nonblocking(fds[1]);
    memset(&srvaddr_o, 0, sizeof(srvaddr_o));
    srvaddr_o.sin_family = AF_INET;
    srvaddr_o.sin_port = htons(PORT_O);
    srvaddr_o.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(fds[1], (struct sockaddr *)&srvaddr_o, sizeof(srvaddr_o));
    //create timer for timerbase
    fds[2] = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if(fds[2] == -1)
    {
		perror("timerfd_create");
		exit(1);
    }
    int i;
    //epoll
    epfd = epoll_create(EVENT_MAX);
    if(epfd == -1)
    {
        perror("epoll_create");
        exit(1);
    }
    int ret;
    for(i = 0; i < 3; i++)
    {
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN | EPOLLET;
        fi = (struct fdinfo*)malloc(sizeof(struct fdinfo));
        if(fi == NULL)
        {
            perror("malloc");
            exit(1);
        }
		fdis[i] = fi;
        if(i == 2)
        {
            fi->flag = 1;
            fi->fd = fds[i];
        }
        else
        {
            fi->flag = 0;
            fi->fd = fds[i];
        }
        ev.data.ptr = fi;
        ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fds[i], &ev);
        if(ret == -1)
        {
            perror("epoll_ctl");
            exit(1);
        }
        debug_info("fds[%d]:%d", i, fds[i]);
    }
#if 1
    struct itimerspec itmp;
    itmp.it_interval.tv_sec = 1;
    itmp.it_interval.tv_nsec = 0;
	itmp.it_value.tv_nsec = 0;
	itmp.it_value.tv_sec = 1;
	ret = timerfd_settime(fds[2],0,&itmp,NULL);
    if(ret == -1)
    {
        perror("timerfd_settime");
        exit(1);
    }
#endif
}
void tq_server_start(void)
{
    server_init();
	//start_diag();
    struct epoll_event events[EVENT_MAX];
    memset(events, 0, sizeof(struct epoll_event)*EVENT_MAX);

    int nfds = 0;
    int i = 0;
    struct fdinfo *fi;
    uint64_t exp;
    int s = 0;
    while(1)
    {
		nfds = epoll_wait(epfd, events, EVENT_MAX, -1);
		if(nfds == -1)
		{
			//perror("epoll_wait");
			exit(1);
		}
        //debug_info("nfds:%d", nfds);
		for(i = 0; i < nfds; i++)
		{
			if(events[i].events & EPOLLIN)
			{
				fi = (struct fdinfo*)((events[i].data.ptr));
				switch(fi->flag)
				{
					case 0:
						debug_info("udp fd:%d", fi->fd);
						if (tq_ser_recv_req(fi->fd) < 0) {
							debug_info("recv data failed!");
						}
						break;
					case 1:
						s = read(fi->fd, &exp, sizeof(uint64_t));
						if(s == -1)
						{
							if (errno == EAGAIN || errno == EINVAL)
							{
								continue;
							}
							else
							{
								continue;
							}
						}
						time_update();
						//debug_info("%d", current_msec);
						expire_timers();
						break;
					default:
						break;
				}
			}
		}
    }
}
