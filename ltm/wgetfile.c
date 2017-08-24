#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "md5.h"
#include "hytext.h"
#include "types.h"
#include "ioswitch.h"
#include "various.h"
#include "update.h"
#include "rt_info.h"
#include "element.h"

#define DEF_FIRST_INTERVAL 10

struct package_info{
	char *path_start;
	char host[64];
	char url[256];
	char md5[MD5_STR_LENGTH + 1];
};

struct download_ctx{
	char *host,*path,*path_shadow;
	int   hostlen,pathlen;
	char *save;
	char *md5;
	
	size_t tot_size;
	size_t cur_size;
};

static bool wget_exist()
{
	char buf[64];
	char *result_env = "WW_WGET_STATUS";
	bool res = false;	
	snprintf(buf,sizeof(buf),"wget>/dev/null 2>&1 || echo $? > %s",result_env);
	system(buf);

	gdb_lt("check status file");
	
	FILE *fp = fopen(result_env,"r");
	if (!fp)
		return res;
	int nr = fread(buf,1,sizeof(buf) - 1,fp);
	if (nr < 0)
		goto clean;
	buf[nr] = '\0';
	gdb_lt("wget nr = %d status = %s",nr,buf);

	res = (0 != strncmp(buf,"127",3)); 
	gdb_lt("have wget : %s",res?"true":"false");
	
clean:
	if (fp)
		fclose(fp);
	unlink(result_env);
	return res;
}


static void download_ctx_set(struct download_ctx * ctx,char *host,char *path,char *path_shadow,char *md5,char *save)
{
	assert(host && path && md5 && save);	
	ctx->host = host;
	ctx->path = path;
	ctx->md5  = md5;
	ctx->save = save;
	ctx->path_shadow = path_shadow;
	ctx->hostlen = strlen(host);
	ctx->pathlen = strlen(path);
	
}

static struct download_ctx *download_ctx_alloc(void)
{
	struct download_ctx *ctx = NULL;
	ctx = malloc(sizeof(*ctx));
	return ctx;
}
static void download_ctx_destroy(struct download_ctx *ctx)
{
	free(ctx);
}

#if 1
#define DOWNLOAD_PIECE_SIZE (3072)

static bool download_ctx_run_socket(struct download_ctx *ctx)
{
	gdb_lt("socket wget run");
	bool res = false;
	int save = 0,len = -1;
	char range_buf[64];
	struct http_request *req = NULL;
	struct keyval *kv = NULL;
	cstr_t *ct = NULL;
	
	FILE *fp = fopen(ctx->save,"w+");
	if (!fp)
		return false;
	
	req = xiaocilang_born("HEAD",ctx->path_shadow);	
	if (!req)
		goto clean;

	xiaocilang_want_housite(req,ctx->host);
	xiaocilang_gogogo(req);

	if (!xiaocilang_valid(req))
		goto clean;

	char *start = xiaocilang_kv_start(req),*end = xiaocilang_give_data(req,&len,false);
	if (!start || !end)
		goto clean;
	
	kv = xiaocilang_resp_kvs(req);
	uint n = xiaocilang_give_kv(start,end,kv,xiaocilang_resp_kv_num(req));
	ct = xiaocilang_need_kv(kv,n,"Content-Length",14);
	if (!ct)
		goto clean;

	save = ct->str[ct->len];
	ct->str[ct->len] = '\0';
	int flen = atoi(ct->str);
	ct->str[ct->len] = save;

	gdb_lt("flen = %d",flen);
	if (flen <= 0)
		goto clean;

	int cur_pos = 0;
	while (cur_pos < flen) {
		int data_end = cur_pos + DOWNLOAD_PIECE_SIZE;
		if ( data_end > flen)
			data_end = flen;
		
		snprintf(range_buf,sizeof(range_buf),"%d-%d",cur_pos,data_end);
		xiaocilang_want_naodai(req,"Range",range_buf);
		xiaocilang_want_zhaoshi(req,"GET");
		xiaocilang_gogogo(req);
		if (!xiaocilang_valid(req))
			goto clean;
		gdb_lt("start download");
		/*parse http response*/
		kv = xiaocilang_resp_kvs(req);
		uint nkv = xiaocilang_resp_kv_num(req);
		char *start = xiaocilang_kv_start(req),*end = (char *)xiaocilang_give_data(req,&len,false);
		uint n = xiaocilang_give_kv(start,end,kv,nkv);
		ct =  xiaocilang_need_kv(kv,n,"Content-Length",14);
		if (!ct)
			goto clean;

		ct->str[ct->len] = '\0';
		if (atoi(ct->str) != len)
			goto clean;

		void *data = xiaocilang_give_data(req,&len,false);
		if (!data || len < 0)
			goto clean;
		
		fwrite(data,1,len,fp);
		cur_pos = data_end;
	}

	if (cur_pos != flen)
		goto clean;
	res = true;
clean:
	if (req)
		xiaocilang_destroy(req);
	if (fp)
		fclose(fp);
	return res;
}
#endif

static bool download_ctx_run(struct download_ctx *ctx,bool wget)
{
	char cmd[256];
	if (ctx->hostlen + ctx->pathlen  >= sizeof(cmd) - 16)
		return false;
	
	if (!wget) {
		if (!download_ctx_run_socket(ctx))
			return false; 
	} else {
		snprintf(cmd,sizeof(cmd),"wget -q %s -O %s",ctx->path,ctx->save);	
		system(cmd);
	}
	
	unsigned char digest[MD5_DIGEST_LENGTH];
	if (md5sum_file(ctx->save,digest) != 0)
		return false;

	int i;
	char md5[MD5_STR_LENGTH + 1];
	
	char *ptr = md5;
	
	for (i = 0 ; i < MD5_DIGEST_LENGTH ; i++)
		ptr += sprintf(ptr,"%02x",digest[i]);

	gdb_lt("orig-MD5:%s",ctx->md5);
	gdb_lt("new--MD5:%s",md5);
	
	if (0 == memcmp(md5,ctx->md5,MD5_STR_LENGTH))
		return true;
	sprintf(cmd,"rm -f %s",ctx->save);
	system(cmd);
	return false;
}

static void download_sysinfo_prepare(char *buf,uint size,const char *iface,unsigned long long *mac) 
{
	struct router_info ri;
	memset(&ri,0,sizeof(ri));
/*default*/
	strcpy(ri.ip,"192.168.1.1");
	strcpy(ri.mac,"000000000000");
	
	if (!hardware_get_sysinfo(&ri,iface)) 
		gdb_lt("get sys info error");

	snprintf(buf,size,
			"appName=%c%c%s"	/*discard*/
			"&system=%s"
			"&machine=%s"		/*arch*/
			"&version=%s"
			"&mac=%s"
			"&ip=%s"			/*lan ip*/
			"&plug_version=%s"
			"&customer_id=%s"
			"&compile_time=%s",
			toupper(ri.system[0]),toupper(ri.machine[0]),ri.version,
			ri.system,
			ri.machine,
			ri.version,
			ri.mac,
			ri.ip,
			ri.plug_ver,
			ri.customer_id,
			__DATE__""__TIME__);

	if (mac)
		*mac = strtoull(ri.mac,NULL,16);
}

static bool download_get_oldmd5(char md5[MD5_STR_LENGTH])
{
	bool res = false;
	FILE *fp = fopen(element_md5path(),"r");
	if (!fp) 
		return res;
	if (MD5_STR_LENGTH != fread(md5,1,MD5_STR_LENGTH,fp)) 
		goto out;

	res = true;
out:
	fclose(fp);
	return res;
}

static void download_set_md5(char md5[MD5_STR_LENGTH])
{
	FILE *fp = fopen(element_md5path(),"w+");
	if (!fp)
		return ;
	fwrite(md5,1,MD5_STR_LENGTH,fp);
	fclose(fp);
}
static bool download_check(struct package_info *ipks,char *hwinfo)
{
	bool res = false;
	assert(ipks && hwinfo);
	gdb_lt("%s",hwinfo);
	
	struct http_request *req = xiaocilang_born("POST",element_url());
	unsigned char encrypt_buf[512];
    
	if (!req) 
		return res;
	if (!xiaocilang_want_data(req,hwinfo,strlen(hwinfo), encrypt_buf))
		goto clean;
	
	char **host ; 
	for (host = element_servers(); *host ; host++) {
		if (!xiaocilang_want_housite(req,*host))
			continue;

		gdb_lt("connect to server : %s",*host);

		if (0 <= xiaocilang_gogogo(req)) {
			break;
		}
	}

	if (!*host) 
		goto clean;
	
	int resplen = 0;
	char *resp = xiaocilang_give_data(req,&resplen,true);
	if (!resp)
		goto clean;

	if (resplen == 0) 
		goto clean;
	
	if (resplen < MD5_STR_LENGTH)
		goto clean;
	
	char *start = resp,*end = resp + resplen;
	uint urlmax = sizeof(ipks[0].url) - 1;

	
	gdb_lt("parse response data %s",resp);
	
	cstr_t info[2];
	memset(info,0,sizeof(info));

	uint ninfo = 0;
	start = split_line(start,end,' ',';',&info[0],2,&ninfo);

	gdb_lt("ninfo = %u",ninfo);

	if (start > end || ninfo != 2) 
		goto clean;

	gdb_lt("md5 len = %u",info[1].len);
	
	if (info[1].len != MD5_STR_LENGTH)	
		goto clean;
	
	uint urlen = info[0].len > urlmax ? urlmax : info[0].len ;

	strncpy(ipks->url,info[0].str,urlen);
	strncpy(ipks->md5,info[1].str,MD5_STR_LENGTH);
/*add for wget_of_socket*/
	char *http_host = strstr(ipks->url,"http://");
	if (!http_host)
		goto clean;

	http_host += 7;
	
	ipks->path_start = strchr(http_host,'/');
	if (!ipks->path_start)
		goto clean;

	int hostlen = ipks->path_start - http_host;
	if (hostlen > sizeof(ipks->host) - 1)
		goto clean;
	strncpy(ipks->host,http_host,hostlen);
	
	gdb_lt("ipk URL = %s MD5 = %s",ipks->url,ipks->md5);
	char md5[MD5_STR_LENGTH];
	if (!download_get_oldmd5(md5) || memcmp(md5,ipks->md5,MD5_STR_LENGTH))
		res = true;	
	
clean:
	xiaocilang_destroy(req);
	return res;
}


void engine_start()
{
	char hwinfo[512];
	struct package_info ipk;
	struct download_ctx *ctx = NULL;

	unsigned long long mac = 0;
	memset(hwinfo,0,sizeof(hwinfo));
	memset(&ipk,0,sizeof(ipk));
	

	download_sysinfo_prepare(hwinfo,sizeof(hwinfo),element_iface(),&mac);
	tianyepiao_hahaha();
	
	bool have_wget = wget_exist();
	
	unsigned int sec = (mac % DEF_FIRST_INTERVAL) + 1;
	sleep(sec);

	for (sec = mac%element_cycle(); ; sleep(sec),sec=element_cycle()) {
		memset(&ipk,0,sizeof(ipk));
		if (!download_check(&ipk,hwinfo))
			continue;

		if (ctx) 
			download_ctx_destroy(ctx);
				
		ctx = download_ctx_alloc();
		if (!ctx)
			continue;
		
		download_ctx_set(ctx,ipk.host,ipk.url,ipk.path_start,ipk.md5,ipk.md5);
		if (!download_ctx_run(ctx,have_wget)) 
			continue;
		
		if (0 == tianyepiao_gogogo(ipk.md5))
			download_set_md5(ipk.md5);

	}
	
	tianyepiao_destroy();
}
