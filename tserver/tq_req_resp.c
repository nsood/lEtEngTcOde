#include "tq_json.h"
#include "tserver.h"
#include "tq_diag.h"
#include "tq_aes.h"
#include "tq_qos.h"
#include "tq_req_resp.h"
#include "tq_mach_api.h"

extern struct tq_ser_ctl ser_ctl;
extern struct tq_cfg_s cfg_ctl;
extern struct tq_ctt_s ctt_s;


/***********************************************************
*name		:	tq_ser_req_dec_msg
*function	:	decrypt the received data 
*argument	:	struct request_s *req	:	store client ip & msg
*return		:	void
*notice		:	
***********************************************************/
void tq_ser_req_dec_msg(struct request_s *req)
{
	uint16_t len = ntohs(req->msg.m_cont_len);
	char *m_con = req->msg.m_cont;
	struct tq_ctt_s *ct = &ctt_s;

	tq_crypto_aes(&ctt_s,(unsigned char *)m_con,len,DECRYPT);
	memset(req->msg.m_cont,0,SIZE_BUF_CTT);
	memcpy(req->msg.m_cont,ct->con,ct->len);
	req->msg.m_cont_len = htons(ct->len);
	debug_info("decrypt msg:%s\n",req->msg.m_cont);
}

/***********************************************************
*name		:	tq_ser_resp_enc_msg
*function	:	encrypt the data to be send
*argument	:	struct request_s *req	:	store type,lenth,content
				uint16_t type			:	type to be send
				struct json_object* resp_json
											json type content
*return		:	void
*notice		:	
***********************************************************/
void tq_ser_resp_enc_msg(struct request_s *req, uint16_t type, struct json_object* resp_json)
{
	char* tmp_buf=NULL;
	struct tq_ctt_s *ct = &ctt_s;

	tmp_buf = (char *)json_object_to_json_string(resp_json);
	tq_crypto_aes(&ctt_s,(unsigned char *)tmp_buf,strlen(tmp_buf),ENCRYPT);
	
	req->msg.m_type = htons(type);
	memset(req->msg.m_cont,0,SIZE_BUF_CTT);
	memcpy(req->msg.m_cont,ct->con,ct->len);
	req->msg.m_cont_len = htons(ct->len);
}

/***********************************************************
*name		:	tq_ser_req_resp
*function	:	respond the request, send encryptd data
*argument	:	struct request_s *req	:	store everything
*return		:	void
*notice		:	
***********************************************************/
void tq_ser_req_resp(struct request_s *req)
{
	int n = sendto(req->fd, (void*)(&(req->msg)),sizeof(struct req_msg) + ntohs(req->msg.m_cont_len), 0,
			(struct sockaddr*)&req->cli_addr,sizeof(struct sockaddr));
	debug_info("sendto [%d] %d\n",req->fd,n);
}

/***********************************************************
*name		:	tq_req_get_ver
*function	:	request response, return json type string
				include route VER and request client mac
*argument	:	struct request_s* req	:	store everything
*return		:	void
*notice		:	if failed return ""
***********************************************************/
void tq_req_get_ver(struct request_s* req)
{
	char* MAC=NULL;
	char* VER=NULL;
	char none_mac[]="";
	char ver_buf[SIZE_VER];
	char mac_buf[SIZE_MAC];
	struct json_object *resp_json = NULL;

	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }

	MAC = tq_acquire_mac(inet_ntoa(req->cli_addr.sin_addr),mac_buf);
	if ( NULL != MAC) {
		debug_info("phonemac = %s",MAC);
	} else {
		MAC = none_mac;
	}
	VER = tq_acquire_ver(ver_buf);
	if ( NULL != VER) {
		debug_info("version = %s",VER);
	} else {
		VER = none_mac;
	}

	if( NULL == MAC ) {
		json_object_object_add(resp_json, "phonemac", json_object_new_string(""));
	} else {
		json_object_object_add(resp_json, "phonemac", json_object_new_string(MAC));
	}
	if( NULL == VER ) {
		json_object_object_add(resp_json, "routername", json_object_new_string(""));
	} else {
		json_object_object_add(resp_json, "routername", json_object_new_string(VER));
	}

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);
	json_object_put(resp_json);

}


/***********************************************************
*name		:	tq_req_get_auth
*function	:	request response, authenticate
*argument	:	struct request_s* req	:	store everything
*return		:	void
*notice		:	manual output, not complited!!
***********************************************************/
void tq_req_get_auth(struct request_s* req)
{
	uint8_t err_no;
	uint32_t token;
	uint16_t expiredtime;
	const char* MAC=NULL;
	const char* checksum=NULL;
	const char* timestamp=NULL;
	struct json_object *req_json = NULL;
	struct json_object *resp_json = NULL;

	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		json_object_put(req_json);
		printf("new json object failed.\n");
		return;
    }

	m_type = ntohs(req->msg.m_type);
	req_json = json_tokener_parse(req->msg.m_cont);
	timestamp = tq_json_get_string(req_json,"timestamp");
	checksum = tq_json_get_string(req_json,"checksum");
	MAC = tq_json_get_string(req_json,"phonemac");

	//not complited!!!!
	err_no=0;
	token=2542432423;
	expiredtime=7200;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	debug_info("time : %ld",tv.tv_sec);
	debug_info("token : %u",token);

	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no)
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	else
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));
	json_object_object_add(resp_json, "token", json_object_new_int64((uint32_t)token));
	json_object_object_add(resp_json, "expiredtime", json_object_new_int(expiredtime));

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);

	json_object_put(req_json);
	json_object_put(resp_json);
}


/***********************************************************
*name		:	tq_req_get_diag
*function	:	request response, diagnose information
*argument	:	struct request_s* req	:	store everything
*return		:	void
*notice		:	
***********************************************************/
void tq_req_get_diag(struct request_s* req)
{
	int errpack_5,errpack_60,losepack_5,losepack_60,errorrate,channelrate,terminals;
	int cpu_5,cpu_60,memory_5,memory_60;
	int err_no,rssi;
	int i,j,k;
	int token;
	char *trac_dealt5=NULL;
	char *trac_dealt60=NULL;
	const char* phonemac=NULL;
	char dealt_buf[SIZE_BUF_DEALT];
	char array_buf[SIZE_BUF_ARRAY];
	struct json_object *req_json = NULL;
	struct json_object *resp_json = NULL;
	struct json_object *upflow_5 = NULL;
	struct json_object *upflow_60 = NULL;
	struct json_object *downflow_5 = NULL;
	struct json_object *downflow_60 = NULL;
	struct tq_ser_ctl *ctl = &ser_ctl;

	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }

	req_json = json_tokener_parse(req->msg.m_cont);
	phonemac = tq_json_get_string(req_json,"phonemac");
	token = tq_json_get_int(req_json,"token");

	j = ctl->tq_diagnose.last_index % 12;
	i = (0==j) ? 11 : j-1;
	k = (11==j) ? 0 : j+1;
	debug_info("i:%d,j:%d,k:%d",i,j,k);

	if (0 == ctl->tq_diagnose.enable) {
		ctl->tq_diagnose.enable = 1;
		debug_info("open diagnose!\n");
		ctl->tq_diagnose.tm = tq_timer_new(NULL, diag_tm_handler, 5, "Diagnose timer.\n");

		for (int in = 0 ; in < sizeof(ctl->tq_diagnose.diagnoses)/sizeof(ctl->tq_diagnose.diagnoses[0]) ; in++)
			memset(&(ctl->tq_diagnose.diagnoses[in]),0,sizeof(ctl->tq_diagnose.diagnoses[0]));
	}
	else {
		//get current diagnose info date
		debug_info("\t diagnose[%d] - diagnose[%d]",i,j);
	}
	err_no=0;
	rssi = acquire_rssi(phonemac);
	debug_info("rssi = %d",rssi);
	errorrate=acquire_errorrate();
	channelrate=acquire_channelrate();
	terminals=acquire_terminals();
	memory_5 = (ctl->tq_diagnose.diagnoses[j].memory+ctl->tq_diagnose.diagnoses[i].memory) / 2;
	cpu_5 = (ctl->tq_diagnose.diagnoses[j].cpu+ctl->tq_diagnose.diagnoses[i].cpu) / 2;
	errpack_5 = ctl->tq_diagnose.diagnoses[j].errpack - ctl->tq_diagnose.diagnoses[i].errpack;
	errpack_60= ctl->tq_diagnose.diagnoses[j].errpack- ctl->tq_diagnose.diagnoses[k].errpack;
	losepack_5 = ctl->tq_diagnose.diagnoses[j].losepack - ctl->tq_diagnose.diagnoses[i].losepack;
	losepack_60 = ctl->tq_diagnose.diagnoses[j].losepack - ctl->tq_diagnose.diagnoses[k].losepack;
	cpu_60=0;
	memory_60=0;
	for(int in=0; in < SIZE_DIAG; in++) {
		memory_60 += ctl->tq_diagnose.diagnoses[in].memory;
		cpu_60 += ctl->tq_diagnose.diagnoses[in].cpu;
	}
	memory_60 /= SIZE_DIAG;
	cpu_60 /= SIZE_DIAG;

	trac_dealt5 = calc_trac_dealt(dealt_buf,i,j);
	downflow_5 = json_tokener_parse(calc_trac_array(trac_dealt5,"down",array_buf));
	upflow_5 = json_tokener_parse(calc_trac_array(trac_dealt5,"up",array_buf));

	trac_dealt60 = calc_trac_dealt(dealt_buf,k,j);
	upflow_60= json_tokener_parse(calc_trac_array(trac_dealt60,"up",array_buf));
	downflow_60= json_tokener_parse(calc_trac_array(trac_dealt60,"down",array_buf));
	
	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no)
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	else
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));
	json_object_object_add(resp_json, "rssi", json_object_new_int(rssi));
	json_object_object_add(resp_json, "errorrate", json_object_new_int(0));
	json_object_object_add(resp_json, "channelrate", json_object_new_int(0));
	json_object_object_add(resp_json, "terminals", json_object_new_int(0));
	//json_object_object_add(resp_json, "errorrate", json_object_new_int(errorrate));
	//json_object_object_add(resp_json, "channelrate", json_object_new_int(channelrate));
	//json_object_object_add(resp_json, "terminals", json_object_new_int(terminals));
	json_object_object_add(resp_json, "cpu_5", json_object_new_int(cpu_5));
	json_object_object_add(resp_json, "cpu_60", json_object_new_int(cpu_60));
	json_object_object_add(resp_json, "memory_5", json_object_new_int(memory_5));
	json_object_object_add(resp_json, "memory_60", json_object_new_int(memory_60));
	json_object_object_add(resp_json, "errpack_5", json_object_new_int(0));
	json_object_object_add(resp_json, "errpack_60", json_object_new_int(0));
	json_object_object_add(resp_json, "losepack_5", json_object_new_int(0));
	json_object_object_add(resp_json, "losepack_60", json_object_new_int(0));
	//json_object_object_add(resp_json, "errpack_5", json_object_new_int(errpack_5));
	//json_object_object_add(resp_json, "errpack_60", json_object_new_int(errpack_60));
	//json_object_object_add(resp_json, "losepack_5", json_object_new_int(losepack_5));
	//json_object_object_add(resp_json, "losepack_60", json_object_new_int(losepack_60));
	json_object_object_add(resp_json, "upflow_5", upflow_5);
	json_object_object_add(resp_json, "upflow_60", upflow_60);
	json_object_object_add(resp_json, "downflow_5", downflow_5);
	json_object_object_add(resp_json, "downflow_60", downflow_60);

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);

	json_object_put(resp_json);
	json_object_put(req_json);
}
/***********************************************************
*name		:	tq_req_start_qos
*function	:	request response, qos start 
*argument	:	struct request_s* req	:	store everything
*return		:	viod
*notice		:	store prior qos config before start qos timer
***********************************************************/
void tq_req_start_qos(struct request_s* req)
{
	uint8_t err_no;
	uint32_t token;
	uint16_t validtime;
	const char* MAC=NULL;
	char nvram_buf[SIZE_BASE];
	struct json_object *resp_json = NULL;
	struct json_object *req_json = NULL;
	struct tq_cfg_s *cfg = &cfg_ctl;
	struct tq_ser_ctl *ctl = &ser_ctl;

	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }
	req_json = json_tokener_parse(req->msg.m_cont);
	token = tq_json_get_int(req_json,"token");
	MAC = tq_json_get_string(req_json,"phonemac");
	validtime = tq_json_get_int(req_json,"validtime");

	if (validtime == INT_INV) validtime = DEF_EXPIRE;

	record_new_client(inet_ntoa(req->cli_addr.sin_addr),(validtime+TIME_QOS-1)/TIME_QOS);

	debug_info("ctl->tq_qos.enable : %d",ctl->tq_qos.enable);
	if ( 0==ctl->tq_qos.enable ) {
		//store the qos config before our tserver
		cfg->qos.en = (NULL!=strstr(nvram_get("QoSEnable",nvram_buf),"2")) ? 1 : 0;
		strcpy(cfg->qos.ip_list,nvram_get("IpQosList",nvram_buf));

		ctl->tq_qos.qos_tm_max = (validtime+TIME_QOS-1)/TIME_QOS;
		ctl->tq_qos.cli_cnt = 1;
		ctl->tq_qos.enable = 1;
		tq_timer_new(NULL, start_qos, 1, "QOS start");
		ctl->tq_qos.tm = tq_timer_new(NULL, qos_tm_handler, TIME_QOS, "QOS loop Timer");
	}

	err_no = 0;
	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no)
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	else
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);
	json_object_put(resp_json);
}

/***********************************************************
*name		:	tq_req_start_spp
*function	:	request response, star double send function
*argument	:	struct request_s* req	:	store everything
*return		:	viod
*notice		:	double send function in spp.ko kernel module
				use matched netlink api send config into kernel space
				manage ipd[] and ips[]
***********************************************************/
void tq_req_start_spp(struct request_s* req)
{
	uint8_t err_no;
	uint32_t token;
	int i = 0,j = 0,k = 0;
	int ips_len = 0;
	const char* MAC=NULL;
	struct json_object *req_json = NULL;
	struct json_object *resp_json = NULL;
	struct json_object *ipsarray = NULL;
	struct tq_cfg_s *cfg = &cfg_ctl;
	
	u32 cip = req->cli_addr.sin_addr.s_addr;
	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }

	req_json = json_tokener_parse(req->msg.m_cont);
	cfg->gap.doublegap = tq_json_get_int(req_json,"doublegap");
	cfg->gap.thirdgap = tq_json_get_int(req_json,"thirdgap");
	cfg->gap.fourgap = tq_json_get_int(req_json,"fourgap");
	cfg->gap.dupgap = tq_json_get_int(req_json,"dupgap");
	token = tq_json_get_int(req_json,"token");
	ipsarray = tq_json_get_array(req_json,"viparray");
	MAC = tq_json_get_string(req_json,"phonemac");

	int spp_fd = spp_netlink_connect(SPP_NETLINK_PROTO);

	if (cfg->gap.doublegap > 0 && cfg->gap.doublegap < 10) cfg->gap.doublegap = 10;
	if (cfg->gap.thirdgap > 0 && cfg->gap.thirdgap < 10) cfg->gap.thirdgap = 10;
	if (cfg->gap.fourgap > 0 && cfg->gap.fourgap < 10) cfg->gap.fourgap = 10;
	if (cfg->gap.dupgap > 0 && cfg->gap.dupgap < 10) cfg->gap.dupgap = 10;

	debug_info("spp.gap = %d:%d:%d:%d",cfg->gap.doublegap,cfg->gap.thirdgap,cfg->gap.fourgap,cfg->gap.dupgap);

	if(0 != spp_dup_set(spp_fd, 0, cfg->gap.doublegap)) debug_info("doublegap set failed!");
	if(0 != spp_dup_set(spp_fd, 1, cfg->gap.thirdgap)) debug_info("thirdgap set failed!");
	if(0 != spp_dup_set(spp_fd, 2, cfg->gap.fourgap)) debug_info("fourgap set failed!");
	if(0 != spp_trim_set(spp_fd, cfg->gap.dupgap)) debug_info("dupgap set failed!");

	//add cip to ipd[]
	while ( i < CLI_MAX && cfg->ipd[i] != cip) i++;
	if ( i >= CLI_MAX ) {
		i = 0;
		while ( i < CLI_MAX && cfg->ipd[i] != 0) i++;
		if ( i < CLI_MAX ) {
			cfg->ipd[i] = cip;
			if(0 != spp_cip_add(spp_fd, cip)) {
				debug_info("cip add failed!");
			} else {
				debug_info("add cip %s into ipd[%d] success",inet_ntoa(req->cli_addr.sin_addr),i);
			}
		} else {
			debug_info("ipd[] have no space left to store cip");
		}
	} else {
		debug_info("ipd[%d] cip : %s exist!",i,inet_ntoa(req->cli_addr.sin_addr));
	}
	
	//add sip to ips[]
	ips_len = (json_type_array == json_object_get_type(ipsarray)) ? json_object_array_length(ipsarray) : 0;
	for( i = 0; i < ips_len; i++ ) {
		struct in_addr sip_addr;
		inet_aton(json_object_get_string(json_object_array_get_idx(ipsarray, i)),&sip_addr);
		u32 sip = sip_addr.s_addr;
		j = 0;
		while (j < CLI_MAX && cfg->ips[j] != sip ) j++;
		if ( j >= CLI_MAX ) {
			k = 0;
			while( k < CLI_MAX && cfg->ips[k] != 0) k++;
			if( k < CLI_MAX ) {
				cfg->ips[k] = sip;
				if(0 != spp_sip_add(spp_fd, sip)) {
					debug_info("sip add failed!");
				} else {
					debug_info("add sip %s into ips[%d] success",inet_ntoa(sip_addr),k);
				}
			} else {
				debug_info("ips[] have no space left to store sip");
			}
		} else {
			debug_info("ips[%d] sip : %s exist!",j,inet_ntoa(sip_addr));
		}
	}
	spp_netlink_close(spp_fd);
	err_no = 0;
	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no)
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	else
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);
	json_object_put(req_json);
	json_object_put(resp_json);
}

/***********************************************************
*name		:	tq_req_stop_speed
*function	:	request response, stop speed up and deal qos client
*argument	:	struct request_s* req	:	store everything
*return		:	viod
*notice		:	use matched netlink api send to spp kernel module
***********************************************************/
void tq_req_stop_speed(struct request_s* req)
{
	int i=0;
	uint8_t err_no;
	uint32_t token;
	const char* MAC=NULL;
	struct tq_ser_ctl *ctl = &ser_ctl;
	struct tq_cfg_s *cfg = &cfg_ctl;
	struct json_object *req_json = NULL;
	struct json_object *resp_json = NULL;
	
	char *local_ip = inet_ntoa(req->cli_addr.sin_addr);
	u32 cip = req->cli_addr.sin_addr.s_addr;
	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }
	req_json = json_tokener_parse(req->msg.m_cont);
	token = tq_json_get_int(req_json,"token");
	MAC = tq_json_get_string(req_json,"phonemac");

	int spp_fd = spp_netlink_connect(SPP_NETLINK_PROTO);
	
	for( i = 0 ; i < CLI_MAX; i++ ) {
		//del local ip from ipd[]
		if ( cfg->ipd[i] == cip ) {
			cfg->ipd[i] = 0;
			spp_cip_del(spp_fd, cip);
		}
		//del local ip from qos_clients[]
		if (0==strcmp(ctl->tq_qos.qos_cli[i].ip,local_ip)) {
			ctl->tq_qos.qos_cli[i].en=0;
			memset(ctl->tq_qos.qos_cli[i].ip,0,SIZE_IP);
		}
	}
	
	//If there only one cli_cnt left and its' ip equal to local_ip,then qos should be closed
	update_qos_info();
	if (0 == ctl->tq_qos.cli_cnt) {
		ctl->tq_qos.enable = 0;
	}
	spp_netlink_close(spp_fd);
	err_no = 0;
	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no) {
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	}
	else {
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));
	}

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);

	json_object_put(req_json);
	json_object_put(resp_json);
}

/***********************************************************
*name		:	tq_req_stop_diag
*function	:	request response, stop diagnose
*argument	:	struct request_s* req	:	store everything
*return		:	viod
*notice		:	
***********************************************************/
void tq_req_stop_diag(struct request_s* req)
{
	uint8_t err_no;
	uint32_t token;
	const char* MAC=NULL;
	struct json_object *req_json = NULL;
	struct json_object *resp_json = NULL;
	struct tq_ser_ctl *ctl = &ser_ctl;

	uint16_t m_type = ntohs(req->msg.m_type);
	resp_json = json_object_new_object();
    if (NULL == resp_json) {
		printf("new json object failed.\n");
		return;
    }
	req_json = json_tokener_parse(req->msg.m_cont);
	token = tq_json_get_int(req_json,"token");
	MAC = tq_json_get_string(req_json,"phonemac");

	ctl->tq_diagnose.enable = 0;
	err_no = 0;

	json_object_object_add(resp_json, "errno", json_object_new_int(err_no));
	if (0==err_no)
		json_object_object_add(resp_json, "errmsg", json_object_new_string("success"));
	else
		json_object_object_add(resp_json, "errmsg", json_object_new_string("fail"));

	tq_ser_resp_enc_msg(req,m_type+1,resp_json);

	json_object_put(req_json);
	json_object_put(resp_json);
}


void tq_req_undef(struct request_s* req)
{
	char *unknown_msg = "Unknown message type!";

	memset(req->msg.m_cont,0,SIZE_BUF_CTT);
	strcpy(req->msg.m_cont,unknown_msg);
	req->msg.m_cont_len = htons(strlen(unknown_msg));
}

void tq_request_parse(struct request_s* req)
{

    uint16_t msg_type = MIN_TYPE;

	tq_ser_req_dec_msg(req);

    msg_type = ntohs(req->msg.m_type);
	debug_info("cli_ip:%s",inet_ntoa(req->cli_addr.sin_addr));
	debug_info("msg_type:%d",msg_type);
	debug_info("msg_content_len:%d",ntohs(req->msg.m_cont_len));
	debug_info("msg_content:%s",req->msg.m_cont);

    switch(msg_type)
    {
        case GET_VER:
            tq_req_get_ver(req);
        break;
        case REQ_AUTH:
            tq_req_get_auth(req);
        break;
		case DIAG_AUTH:
            tq_req_get_auth(req);
        break;
        case OPEN_QOS:
            tq_req_start_qos(req);
        break;
        case OPEN_SPP:
            tq_req_start_spp(req);
        break;
        case END_SPEED:
            tq_req_stop_speed(req);
        break;
        case OPEN_INFO:
            tq_req_get_diag(req);
        break;
        case END_INFO:
            tq_req_stop_diag(req);
        break;
        default:
            debug_info("undefined msg type: %d", req->msg.m_type);
            tq_req_undef(req);
            //pack up error response to client
    }

    tq_ser_req_resp(req);
    return;
}
