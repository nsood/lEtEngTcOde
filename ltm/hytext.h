#ifndef _HYTEXT_H
#define _HYTEXT_H

#include <stdbool.h>
#include "various.h"
struct http_request ;

struct keyval{
	cstr_t key,val;
};


/*http_request_alloc ---> xiaocilang_born*/
struct http_request *xiaocilang_born(char *method,const char *url);
/*http_request_set_data --> xiaocilang_want_data */
bool xiaocilang_want_data(struct http_request *req,void *data,int dlen, void *encrypt_buf);
/*http_request_clear --> xiaocilang_clear*/
void xiaocilang_clear(struct http_request *req);
/*http_request_destroy --> xiaocilang_dead */
void xiaocilang_destroy(struct http_request *req);
/*http_request_set_header --> xiaocilang_want_naodai*/
bool xiaocilang_want_naodai(struct http_request *req,char *key,char *val);
/*http_request_set_host --> xiaocilang_want_housite*/
bool xiaocilang_want_housite(struct http_request *req,char *host);
/*http_request_del_header  --> xiaocilang_buyao_naodai*/
void xiaocilang_buyao_naodai(struct http_request *req,char *key);
/*http_request_set_method --> xiaocilang_want_zhaoshi*/
void xiaocilang_want_zhaoshi(struct http_request *req,char *method);
/*http_request_run  --> xiaocilang_gogogo*/
int xiaocilang_gogogo(struct http_request *req);
/*http_response_get_data --> xiaocilang_give_data*/
uint xiaocilang_give_kv(char *start,char *end,struct keyval *lines,uint n);
cstr_t *xiaocilang_need_kv(struct keyval *lines ,uint n,char *key,uint len);
struct keyval *xiaocilang_resp_kvs(struct http_request *req);
uint xiaocilang_resp_kv_num(struct http_request *req);
int xiaocilang_resp_dlen(struct http_request *req);
bool xiaocilang_valid(struct http_request *req);
char *xiaocilang_kv_start(struct http_request *req);
void  *xiaocilang_give_data(struct http_request *req,int *len,bool enc);

#endif
