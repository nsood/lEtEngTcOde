#ifndef TIMERS_H_
#define TIMERS_H_
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "rb_tree.h"

typedef rbtree_key_uint_t msec_uint_t;
typedef rbtree_key_int_t msec_int_t;

extern volatile msec_uint_t current_msec;
msec_uint_t time_update(void);

typedef struct event_s timer_event_t;
typedef void(*handler_pt)(timer_event_t *ev);//

#define TIMER_INFO_SIZE 256
#define TIMER_INFINITE  (msec_uint_t) - 1
#define TIMER_LAZY_DELAY        2

struct event_s
{
	rbtree_node_t timer;
	handler_pt handler;
	char data[TIMER_INFO_SIZE];
};

int32_t timer_init(void);
msec_uint_t find_timer(void);
void expire_timers(void);
int32_t no_timers_left(void);
void del_timer(timer_event_t *ev);
void add_timer(timer_event_t *ev, msec_uint_t msec);
rbtree_node_t tq_timer_new(timer_event_t *arg,void (*func)(timer_event_t *ev),int s,char *dbg_info);
void free_all_timer();

extern rbtree_t timer_rbtree;

#endif//timers.h
