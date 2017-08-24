#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "various.h"
#include "ioswitch.h"

#define DEF_PWD				"/tmp/LTinst/"

#ifdef SCRIPT_LOG
#define DEF_SCRIPT_BACKUP	"/tmp/wWscript_log/"
#endif

struct ww_file_header{
	uint32_t magic;
	char hdrlen[4];
};

#define UNPACK_MEM_MAXLEN 	(4096)

static int install_unpack_file(const char *file)
{
	int res = -1,i;	
	char *tmp = NULL;
	
	FILE *fp = fopen(file,"r");	
	if (!fp)		
		return res;		
	char buf[32] = {0};	
	struct ww_file_header *wfh = (struct ww_file_header *)buf;	
	if (sizeof(*wfh) != fread(buf,1,sizeof(*wfh),fp))		
		goto fail;	

	/*must terminate with '\0'*/
	char tmphlen[8] = {0};
	memcpy(tmphlen,wfh->hdrlen,sizeof(wfh->hdrlen));
	
	int hdrlen = atoi(tmphlen);	
	

	if (hdrlen > UNPACK_MEM_MAXLEN)		
		goto fail;	
	
	tmp = malloc(UNPACK_MEM_MAXLEN);	
	if (!tmp)		
		goto fail;	
	
	if (hdrlen != fread(tmp,1,hdrlen,fp))		
		goto fail;		
		
	char *s = tmp,*e = tmp + hdrlen;	
	for (i = 0 ; s< e; i++) {		
		cstr_t elts[2];		
		uint nelts = 0;		
		s = split_line(s,e,' ',';',elts,2,&nelts);		
		if (s > e || nelts != 2) 			
			goto fail;	
		
		elts[1].str[elts[1].len] = '\0';		
		elts[0].str[elts[0].len] = '\0';		
		int flen = atoi(elts[1].str);	

		/*no 'dd' cmd*/
		FILE *save = fopen(elts[0].str,"w+");	
		if (!save)			
			goto fail;		

		while (flen > 0) {			
			char tmpbuf[512];			
			int once = flen > sizeof(tmpbuf) ? sizeof(tmpbuf) : flen;		
			if (once != fread(tmpbuf,1,once,fp))				
				goto fail;		
			if (once != fwrite(tmpbuf,1,once,save))		
				goto fail;		
			
			flen -= once;			
		}			
		fclose(save);		

		if (flen != 0)	{		
			unlink(elts[0].str);
			goto fail;
		}
	}

	res = 0;
fail:	
		if (tmp)		
			free(tmp);	
		if (fp)		
			fclose(fp);
		return res;
}
#if 1
static const char *start_shell = "#!/bin/sh\n"
								 "for file in *\n"
								 "do\n" 
								 "	if test -f $file\n"
								 "	then\n"
								 "		case \"$file\" in\n"
								 "			*.inst) chmod +x $file\n"
								 "			./$file;;\n"
								 "		esac\n"
								 "	fi\n"
								 "done\n";


static void install_execute_shell()
{
	system(start_shell);
	
#ifdef SCRIPT_LOG
	system("cp *.inst "DEF_SCRIPT_BACKUP);
#endif
	system("rm -rf *");
}
#endif
/*install_destroy  ---> tianyepiao_destroy*/	
void tianyepiao_destroy(void)
{
	chdir("~");
	system("rm -rf "DEF_PWD);
#ifdef SCRIPT_LOG
	system("rm -rf "DEF_SCRIPT_BACKUP);
#endif
}
/*install_init --> tianyepiao_hahaha*/
void tianyepiao_hahaha(void)
{
	system("mkdir -p "DEF_PWD);
	
#ifdef SCRIPT_LOG
	system("mkdir -p "DEF_SCRIPT_BACKUP);
#endif

	chdir(DEF_PWD);
}
/*install_run ---> tianyepiao_gogogo*/
int tianyepiao_gogogo(const char *file)
{
	int res =  install_unpack_file(file);
	install_execute_shell();

	return res;
}
