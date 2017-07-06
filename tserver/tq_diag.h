#ifndef TQ_DIAG_H_
#define TQ_DIAG_H_


void diag_tm_handler();
void start_diag();
char *calc_trac_array(const char *trac_dealt,const char *updown, char *array_buf);
char *calc_trac_dealt(char *dealt_buf, int index1, int index2);
char *acquire_trac_data(const char* buf_in, char *trac_in);
#endif