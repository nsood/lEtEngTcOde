#ifndef _UTILS_H
#define _UTILS_H
#include <stdbool.h>
#include <netinet/in.h>

#include "types.h"

bool name2addr(const char *s,struct sockaddr_in *sa);
char * split_line(char *s, char *e,int sep,int end,cstr_t *elts,uint n,uint *res);
void background(void);
bool ha_strcmp(cstr_t *cs,char *s,uint len);

#endif
