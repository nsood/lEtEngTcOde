#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "types.h"
#include "various.h"
#include "ioswitch.h"
#include "lt_endecry.h"
#include "hytext.h"

#define DEF_RESP_SIZE       (4096)
#define DEF_REQ_SIZE		(2048)
#define DEF_REQ_PORT		(8080)

struct http_header_line{
	char *key,*val;
	struct list_head next;
};
struct http_response {
	bool valid;
	char *line_start;
	struct keyval lines[16];
	void *data;
	int dlen;
	char buf[DEF_RESP_SIZE];
};

struct http_request {
	const char *host;
	const char *method;
	const char *url;
	const char *version;
	struct list_head lines;
	int nline;
	
	void *data;
	int   dlen;
	char buf[DEF_REQ_SIZE];

	struct sockaddr_in sa;
	struct http_response resp;
};

static char *http_trisection_header(char *start,char *end,cstr_t *result3)
{
	uint i;
	
	for (i = 0; i < 3 ; i++) {
		ha_strip_space(start,end);
		result3[i].str = start;
		ha_grab_text(start,end);
		result3[i].len = start - result3[i].str;
	}
	return start;
}

static struct http_header_line *http_header_line_find(struct http_request *req,char *key)
{
	struct http_header_line *pos;
	list_for_each_entry(pos,&req->lines,next) {
		if (pos->key && 0 == strcmp(pos->key,key))
			return pos;
	}
	return NULL;
}


struct http_request *xiaocilang_born(char *method,const char *url)
{
	assert(method && url);
	struct http_request *req ;
	req = calloc(1,sizeof(*req));
	if (!req)
		return req;
	req->method = method;
	req->url = url;
	req->version = "HTTP/1.0";
	req->nline = 0;
	req->data = NULL;
	req->dlen = 0;

	req->resp.valid = false;
	INIT_LIST_HEAD(&req->lines);

	return req;
}
bool xiaocilang_want_data(struct http_request *req,void *data,int dlen, void *encrypt_buf)
{    
    if (0 == encrypt_with_default_pass(data, dlen, encrypt_buf)) 
    {
		req->data = encrypt_buf;
		req->dlen = dlen;
        return true;
	}

    return false;
}
void xiaocilang_clear(struct http_request *req)
{
	assert(req);
	req->method = req->url = req->version = NULL;
	struct http_header_line *pos,*_next;
	list_for_each_entry_safe(pos,_next,&req->lines,next) {
		list_del(&pos->next);
		free(pos);
	}
	
	req->resp.valid = false;
	req->nline = 0;
}
void xiaocilang_destroy(struct http_request *req)
{
	xiaocilang_clear(req);
	free(req);
}
bool xiaocilang_want_naodai(struct http_request *req,char *key,char *val)
{
	assert(req && key && val);
	struct http_header_line *line = http_header_line_find(req,key);
	if (line) {
		line->val = val;
		return true;
	}
	
	line = malloc(sizeof(*line));
	if (!line)
		return false;

	line->key = key;
	line->val = val;
	INIT_LIST_HEAD(&line->next);
	list_add_tail(&line->next,&req->lines);
	req->nline++;
	return true;
}
bool xiaocilang_want_housite(struct http_request *req,char *host)
{
	assert(req && host);
	if (!name2addr(host,&req->sa))
		return false;
	
	req->host = host;
	
	req->sa.sin_family = AF_INET;
	req->sa.sin_port = htons(DEF_REQ_PORT);
	
	return xiaocilang_want_naodai(req,"Host",host);
}
void xiaocilang_buyao_naodai(struct http_request *req,char *key)
{
	assert(req && key);
	struct http_header_line *line = http_header_line_find(req,key);
	if (!line)
		return ;
	list_del(&line->next);
	free(line);
	req->nline--;
}
void xiaocilang_want_zhaoshi(struct http_request *req,char *method)
{
	assert(req && method);
	req->method = method;
}
int xiaocilang_gogogo(struct http_request *req)
{
	assert(req);

	int res = -1;
	struct iovec iov[3];
	int len = 0,niov = 1;
		
	char *pbuf = req->buf,*end = req->buf + sizeof(req->buf);
	pbuf += snprintf(pbuf,end - pbuf,"%s %s %s\r\n",req->method,req->url,req->version);

	struct http_header_line *pos;
	list_for_each_entry(pos,&req->lines,next) {
		if (end > pbuf) {
			pbuf +=  snprintf(pbuf,end - pbuf,"%s:%s\r\n",pos->key,pos->val);
			continue;
		} 
		gdb_lt("need more buf to send request");
		return res;
	}
	
	iov[0].iov_base = req->buf;
	
	if (req->data && req->dlen > 0) {
		pbuf += snprintf(pbuf,end - pbuf,
			"Content-Type:application/x-www-form-urlencoded\r\nContent-Length:%d\r\n\r\n",req->dlen);
		
		iov[1].iov_base = req->data;
		iov[1].iov_len = req->dlen;
		len += req->dlen;
		niov++;
	} else {
		pbuf += snprintf(pbuf,end - pbuf,"Content-Length:0\r\n\r\n");
	}
		
	iov[0].iov_len = pbuf - req->buf;
	len += iov[0].iov_len;

	
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0)
		return res;

	int tries = 5;
	while (--tries > 0 && connect(sock,(struct sockaddr *)&req->sa,sizeof(req->sa))) {
		sleep(3);
	}

	if (tries == 0) {
		gdb_lt("connect failed");
		goto clean;
	}
	
	if (len != writev(sock,iov,niov)) 
		goto clean;

	ssize_t nr = recv(sock,req->resp.buf,sizeof(req->resp.buf),0);
	if (nr <= 0) {
		gdb_lt("no response");
		goto clean;
	}

	req->resp.buf[nr] = '\0';
	
	gdb_lt("response  %s",req->resp.buf);
	
	char *start;
	start = req->resp.buf;
	end = start + nr;
	cstr_t result[3];
	start = http_trisection_header(start,end,result);
	if (result[1].len != 3 || strncmp(result[1].str,"200",3))
		goto clean;

	ha_strip_space(start,end);
	if (start >= end)
		goto clean;
	
	req->resp.line_start = strstr(start,"\r\n");
	if (req->resp.line_start) {
		req->resp.line_start += 2;
		start = req->resp.line_start;
	}
	
	char *data = strstr(start,"\r\n\r\n");
	if (!data)
		goto clean;

	req->resp.data = data + 4;
	req->resp.dlen = end - data - 4;
	req->resp.valid = true;
	
	gdb_lt("dlen %d",req->resp.dlen);
	res = req->resp.dlen ;
clean:
	close(sock);
	return res;
}
/*http_response_get_data --> xiaocilang_give_data*/
void  *xiaocilang_give_data(struct http_request *req,int *len,bool enc)
{
	void *res = NULL;
    
	assert(req && len);
	if (!req->resp.valid)
		return res;
	
    if (enc && 0 != decrypt_with_default_pass(req->resp.data, req->resp.dlen, req->resp.data)) 
		return res;

	if (enc)
    	gdb_lt("after decrpyt : %s",req->resp.data);
	
   	res = req->resp.data;
	*len = req->resp.dlen;
	return res;
}

#if 1

uint xiaocilang_give_kv(char *start,char *end,
								struct keyval *lines,uint n)
{
	uint i;
	/*Fix:strip space*/
	for (i = 0 ; start < end && i < n ; i++) {
		char *sep = memchr(start,':',end - start);
		if (!sep)
			goto done;
		
		ha_strip_space(start,sep);
		lines[i].key.str = start;
		ha_grab_text(start,sep);
		lines[i].key.len = start - lines[i].key.str;
		
		start = ++sep;
	
		ha_strip_space(start,end);
		lines[i].val.str = start;
//		ha_grab_text(start,end);
		ha_skip_newline(start,end);
		if (*(start - 1) == '\r')
			start--;
		
		lines[i].val.len = start - lines[i].val.str;
	}
done:
	return i;
}

cstr_t *xiaocilang_need_kv(struct keyval *lines ,uint n,char *key,uint len)
{
	uint nline ;
	for (nline = 0 ; nline < n ; nline++) {
		if (ha_strcmp(&lines[nline].key,key,len)) 
			return &lines[nline].val;
	}
	return NULL;
}
struct keyval *xiaocilang_resp_kvs(struct http_request *req)
{
	return req->resp.lines;
}
uint xiaocilang_resp_kv_num(struct http_request *req)
{
	return sizeof(req->resp.lines)/sizeof(req->resp.lines[0]);
}

int xiaocilang_resp_dlen(struct http_request *req)
{
	return req->resp.dlen;
}
bool xiaocilang_valid(struct http_request *req)
{ return req->resp.valid;}

char *xiaocilang_kv_start(struct http_request *req)
{ return req->resp.line_start;}
#endif

