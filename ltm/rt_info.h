#ifndef _RT_INFO_H
#define _RT_INFO_H

#include <sys/utsname.h>
#include <stdbool.h>


#ifndef _UTSNAME_MACHINE_LENGTH
    #ifdef _UTSNAME_LENGTH
        #define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH
        #define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
    #else
        #error "undefine _UTSNAME_MACHINE_LENGTH & _UTSNAME_VERSION_LENGTH"
    #endif
#endif


struct router_info{
	char machine[_UTSNAME_MACHINE_LENGTH];
	char version[_UTSNAME_VERSION_LENGTH];
	char system[64];
	char mac[16];
	char ip[20];
	char plug_ver[16];
	char customer_id[16];
};

bool hardware_get_sysinfo(struct router_info *ri,const char *iface);
#endif
