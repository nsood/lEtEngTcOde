#include "include.h"
#include "tserver.h"
#include "tq_diag.h"
#include "tq_json.h"
#include "tq_mach_api.h"

extern struct tq_ser_ctl ser_ctl;

/***********************************************************
*name		:	acquire_trac_data
*function	:	acquire traffic information
*argument	:	
*return		:	point of json type string
*notice		:	
***********************************************************/
char *acquire_trac_data(const char* buf_in, char *trac_in)
{
	int i=0,in=0;
	int j=0,jn=0;
	char *p[20][50];
	char *out_p=NULL;
	char *in_p=NULL;
	char *buf_out=NULL;
	struct json_object *trac = NULL;
	trac = json_object_new_array();
	
	char buf[SIZE_BUF_TRAC];
	strcpy(buf,buf_in);
	char *buf_p=&buf[0];
	while((p[i][j]=strtok_r(buf_p,"\n",&out_p))!=NULL) {
		buf_p = p[i][j];
		while((p[i][j]=strtok_r(buf_p," ",&in_p))!=NULL) {
			buf_p = NULL;
			jn=++j;
		}
		buf_p = NULL;
		j=0;
		in=++i;
	}

	for (i=0;i<in;i++){
		struct json_object *host = json_object_new_object();
		for(j=0;j<jn;j++) {
			if (j==19) json_object_object_add(host, "down", json_object_new_int(atoi(p[i][j])/1024/1024));
			if (j==5) json_object_object_add(host, "up", json_object_new_int(atoi(p[i][j])/1024/1024));
			if (j==2) json_object_object_add(host, "ip", json_object_new_string(p[i][j]));
		}
		json_object_array_add(trac, host);
	}
	buf_out = (char*)json_object_to_json_string(trac);
	if (NULL == buf_out){
		json_object_put(trac);
		return NULL;
	} else {
		strcpy(trac_in,buf_out);
	}
	json_object_put(trac);
	return trac_in;
}

/***********************************************************
*name		:	calc_trac_dealt
*function	:	calculate the dealt traffic value
*argument	:	int dup		:	interval
				int index1	:	first string array index
				int index2	:	second string array index
*return		:	json type string dealt traffic
*notice		:	
***********************************************************/
char *calc_trac_dealt(char *dealt_buf, int index1, int index2)
{
	int i,j,s1_len,s2_len,dealt_upflow,dealt_downflow;
	struct json_object *s1_json=NULL;
	struct json_object *s2_json=NULL;
	struct json_object *dealt_json=NULL;
	char s1[SIZE_BUF_TRAC]={'\0'};
	char s2[SIZE_BUF_TRAC]={'\0'};
	struct tq_ser_ctl *ctl = &ser_ctl;
	dealt_json = json_object_new_array();
    if (NULL == dealt_json) {
		printf("new json object failed.\n");
    }

	if (NULL == acquire_trac_data(ctl->tq_diagnose.diagnoses[index2].traffic,s2)) {
		s2_json = json_tokener_parse("[]");
	} else {
		s2_json = json_tokener_parse(s2);
	}
	if (NULL == acquire_trac_data(ctl->tq_diagnose.diagnoses[index1].traffic,s1)) {
		s1_json = json_tokener_parse("[]");
	} else {
		s1_json = json_tokener_parse(s1);
	}

	s2_len = (json_type_array == json_object_get_type(s2_json)) ? json_object_array_length(s2_json) : 0;
	s1_len = (json_type_array == json_object_get_type(s1_json)) ? json_object_array_length(s1_json) : 0;
	//debug_info("s1_len=%d,s2_len=%d",s1_len,s2_len);
	for(j=0; j<s2_len; j++) {
		int new_flag=0;
		struct json_object *obj2 = json_object_array_get_idx(s2_json, j);
		for(i=0; i<s1_len; i++) {
			struct json_object *obj1 = json_object_array_get_idx(s1_json, i);
			if (0==strcmp(tq_json_get_string(obj1,"ip"),tq_json_get_string(obj2,"ip"))){
				struct json_object *host_json = json_object_new_object();
				//down flow traffic
				if (tq_json_get_int(obj2,"down")>tq_json_get_int(obj1,"down"))
					dealt_downflow = (tq_json_get_int(obj2,"down")-tq_json_get_int(obj1,"down"));
				else
					dealt_downflow = 0;
				//up flow traffic
				if(tq_json_get_int(obj2,"up")>tq_json_get_int(obj1,"up"))
					dealt_upflow = (tq_json_get_int(obj2,"up")-tq_json_get_int(obj1,"up"));
				else
					dealt_upflow = 0;
				json_object_object_add(host_json,"down",json_object_new_int(dealt_downflow));
				json_object_object_add(host_json,"up",json_object_new_int(dealt_upflow));
				json_object_object_add(host_json,"ip",json_object_new_string(tq_json_get_string(obj1,"ip")));
				json_object_array_add(dealt_json,host_json);
				new_flag = 1;
				break;
			}
		}
		if (1==new_flag){
			new_flag = 0;
		} else {
			//new host traffic
			json_object_array_add(dealt_json,obj2);
		}
	}
	char *j_s = (char*)json_object_to_json_string(dealt_json);
	strcpy(dealt_buf,j_s);
	json_object_put(s2_json);
	json_object_put(s1_json);
	json_object_put(dealt_json);
	return dealt_buf;
}

/***********************************************************
*name		:	calc_trac_array
*function	:	make the special array format 'x.x.x.x:xxx:xxx' array
*argument	:	const char *trac_dealt	:	json type traffic dealt string
				const char *updown		:	'up' or 'down'
*return		:	result json array type string point
*notice		:	
***********************************************************/
char *calc_trac_array(const char *trac_dealt,const char *updown, char *array_buf)
{
	int i=0,lenth=0;
	char flow[10];
	char ip_info[20];
	struct json_object *dealt_json=NULL;
	struct json_object *array = json_object_new_array();
	debug_info("input s=%s",trac_dealt);
	char buf_in[SIZE_BUF_DEALT];
	memcpy(buf_in,trac_dealt,strlen(trac_dealt)+1);
	if (0==strcmp(buf_in,""))
		dealt_json = json_tokener_parse("[]");
	else
		dealt_json = json_tokener_parse(buf_in);
	lenth = (json_type_array == json_object_get_type(dealt_json)) ? json_object_array_length(dealt_json) : 0;
	//debug_info("trac_dealt lenth=%d",lenth);
	for(i=0; i<lenth; i++) {
		struct json_object *obj = json_object_array_get_idx(dealt_json, i);
		//debug_info("each host info :\n%s",json_object_to_json_string(obj));
		memset(ip_info,0,20);
		strcpy(ip_info,tq_json_get_string(obj,"ip"));
		strcat(ip_info,":");
		sprintf(flow,"%d",tq_json_get_int(obj,updown));
		strcat(ip_info,flow);
		debug_info("ip:flow = %s",ip_info);
		json_object_array_add(array,json_object_new_string(ip_info));
	}
	char *buf = (char*)json_object_to_json_string(array);
	memset(array_buf,0,SIZE_BUF_ARRAY);
	strcpy(array_buf,buf);
	debug_info("output s=%s",array_buf);
	json_object_put(array);
	json_object_put(dealt_json);
	return array_buf;
}

/***********************************************************
*name		:	diag_tm_handler
*function	:	diagnose timeout handler, calculate diagnose information
*argument	:	void
*return		:	void
*notice		:	data store in static variables
***********************************************************/
void diag_tm_handler()
{
	struct tq_ser_ctl *ctl = &ser_ctl;
	struct timespec tm = {0, 0};
	clock_gettime(CLOCK_MONOTONIC ,&tm);
	int i = ctl->tq_diagnose.last_index;
	i=(i==11)?0:i+1;
	ctl->tq_diagnose.diagnoses[i].losepack = acquire_losepack();
	ctl->tq_diagnose.diagnoses[i].memory = acquire_memory();
	ctl->tq_diagnose.diagnoses[i].cpu = acquire_cpu();
	ctl->tq_diagnose.diagnoses[i].errpack = acquire_errpack();
	ctl->tq_diagnose.diagnoses[i].time = tm.tv_sec;
	
	sys_popen("cat /proc/net/ipt_account/iptraffic",ctl->tq_diagnose.diagnoses[i].traffic,SIZE_BUF_TRAC);
	//debug_info("traffic[%d]",i);

	if (1 == ctl->tq_diagnose.enable) {
		ctl->tq_diagnose.last_index = i;
		ctl->tq_diagnose.tm = tq_timer_new(NULL, diag_tm_handler, 5, "diagnose timeout");
	}
}

/***********************************************************
*name		:	start_diag
*function	:	interface to open diagnose timer and start collecting info
*argument	:	void
*return		:	void
*notice		:	
***********************************************************/
void start_diag()
{
	struct tq_ser_ctl *ctl = &ser_ctl;
	ctl->tq_diagnose.enable = 1;
	debug_info("open diagnose!");
	struct timespec tm = {0, 0};
	clock_gettime(CLOCK_MONOTONIC ,&tm);
	int i = (tm.tv_sec % SEC_60) / SEC_5;
	ctl->tq_diagnose.last_index = i;
	//debug_info("init index : %d",i);
	ctl->tq_diagnose.tm = tq_timer_new(NULL, diag_tm_handler, 5, "Diagnose timer");

	for (int i = 0 ; i < sizeof(ctl->tq_diagnose.diagnoses)/sizeof(ctl->tq_diagnose.diagnoses[0]) ; i++)
		memset(&(ctl->tq_diagnose.diagnoses[i]),0,sizeof(ctl->tq_diagnose.diagnoses[0]));
}

