#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

static bool io_value = false;

#if 1
static FILE *gdb_fp = NULL;
#else
#define gdb_fp stderr;
#endif

#define GDB_FILE_PATH "/tmp/ltm.log"

void ioctl_stdout(const char *func,int line,const char *fmt,...)
{
	if (!io_value)
		return ;
	
	if (!gdb_fp)
		gdb_fp = fopen(GDB_FILE_PATH,"w+");
		if (!gdb_fp)
			return ;
	
    fprintf(gdb_fp,"<%20s>[%03d] : ",func,line);
    va_list ap; 
    va_start(ap,fmt);
    vfprintf(gdb_fp,fmt,ap);
    va_end(ap);
    fprintf(gdb_fp,"\n");
	fflush(gdb_fp);
}


void ioctl_switch(void)
{
	io_value = true;
}


