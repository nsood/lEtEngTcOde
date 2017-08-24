#ifndef _TYPES_H
#define _TYPES_H
#include <ctype.h>

typedef unsigned int uint;

typedef struct _const_str_s{
	char *str;
	uint len;
}cstr_t;


#define ha_string(_x_) {_x_,sizeof(_x_) - 1}
#define ha_strip_space(_start_,_end_)		do {for ( ; _start_ < _end_ && isspace(*_start_) ; _start_++);}while(0)
#define ha_strip_space_tail(_start_,_end_)  do {for ( ; _start_ > _end_ && isspace(*_end_) ; _end_--);}while(0)
#define ha_grab_text(_start_,_end_)  		do {for ( ; _start_ < _end_ && !isspace(*_start_) ; _start_++);}while(0)
#define ha_skip_newline(_start_,_end_) 		do{for (;_start_ < _end_ && *_start_ != '\n';_start_++);}while(0)

#endif
