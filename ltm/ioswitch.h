#ifndef _IOSWITCH_H
#define _IOSWITCH_H


void ioctl_stdout(const char *func,int line,const char *fmt,...);
void ioctl_switch(void);
#define gdb_lt(_fmt,...)	 ioctl_stdout(__func__,__LINE__,_fmt,##__VA_ARGS__)
    

#endif
