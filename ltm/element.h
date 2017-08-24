#ifndef _ELEMENT_H
#define _ELEMENT_H

void element_record_vendor(char *c);
void element_record_iface(char *ifname);
void element_record_md5(char *m);
void element_record_server(char **s);
void element_record_cycle(int i);
void element_record_url(char *url);
void element_record_plugintime(void);

int element_cycle(void);
const char *element_board(void);
const char *element_vendor(void);
const char *element_iface(void);
const char *element_md5path(void);
const char *element_url(void);
char **element_servers(void);


#endif
