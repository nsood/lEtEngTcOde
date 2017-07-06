#ifndef TQ_QOS_H_
#define TQ_QOS_H_

int acquire_ip_qos_list(char *ip_qos_list,size_t size);
void start_qos();
void record_new_client(char *cip,int times);
void update_qos_info();
void qos_tm_handler();

#endif