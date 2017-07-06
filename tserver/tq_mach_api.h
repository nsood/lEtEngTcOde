#ifndef TQ_MACH_API_H_
#define TQ_MACH_API_H_

char *sys_popen(const char *cmd, char *result, int m_len);
char *nvram_get(const char *name, char *nvram_buf);
void nvram_set(const char *name, const char *args);
char *tq_acquire_ver(char *v);
char *tq_acquire_mac(char *ip, char *mac_buf);
int acquire_cpu();
int acquire_memory();
int acquire_errorrate();
int acquire_channelrate();
int acquire_terminals();
int acquire_errpack();
int acquire_losepack();
int acquire_rssi(const char *mac);

#endif