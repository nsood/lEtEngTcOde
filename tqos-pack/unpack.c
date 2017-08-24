#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define UNPACK_MEM_MAXLEN 	(4096)

struct ww_file_header{
	uint32_t magic;
	char hdrlen[4];
};

typedef struct _const_str_s{
	char *str;
	uint len;
}cstr_t;

static char *split_line(char *s, char *e,int sep,int end,cstr_t *elts,uint n,uint *res)
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
static int install_unpack_file(const char *file,const char *prefix)
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
		char newpath[256];
		sprintf(newpath,"%s%s",prefix,elts[0].str);
		FILE *save = fopen(newpath,"w+");	
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


#define UNPACK_PATH "unpack-tmp/"

int main(int argc,char **argv)
{
	system("rm -rf "UNPACK_PATH);
	system("mkdir -p "UNPACK_PATH);

	char obs_path[256];

	char *source = "wWrelease.bin";

	if (argc == 2)
		source = argv[1];

	install_unpack_file(source,UNPACK_PATH);

	return 0;
}
