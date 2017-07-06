#include "timers.h"
#include "debug.h"
#include <stddef.h>
#include <stdio.h>

rbtree_t timer_rbtree;
static rbtree_node_t timer_sentinel;

volatile msec_uint_t current_msec;

msec_uint_t time_update(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t sec;
    msec_uint_t msec;

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
    current_msec = (msec_uint_t)sec;
	return current_msec;
}

int32_t timer_init()
{
    rbtree_init(&timer_rbtree, &timer_sentinel, rbtree_insert_timer_value);
    return 0;
}
msec_uint_t find_timer(void)
{
    msec_int_t timer;
    rbtree_node_t *node, *root, *sentinel;
    if(timer_rbtree.root == &timer_sentinel)
    {
        return TIMER_INFINITE;
    }
    root = timer_rbtree.root;
    sentinel = timer_rbtree.sentinel;
    node = rbtree_min(root, sentinel);
    timer = (msec_int_t)(node->key - current_msec);
    return (msec_uint_t)(timer > 0 ? timer : 0);
}
void expire_timers(void)
{
    timer_event_t *ev;
    rbtree_node_t *node, *root, *sentinel;
    sentinel = timer_rbtree.sentinel;
    while(1)
    {
		root = timer_rbtree.root;
		if(root == sentinel)
		{
			return;
		}
		node = rbtree_min(root, sentinel);
		if((msec_int_t)(node->key - current_msec) > 0)
		{
			return;
		}
		ev = (timer_event_t*)((char*)node - offsetof(timer_event_t, timer));
		rbtree_delete(&timer_rbtree,&ev->timer);
		ev->handler(ev);
		free(ev);
		ev = NULL;
    }
}
void del_timer(timer_event_t *ev)
{
    rbtree_delete(&timer_rbtree, &ev->timer);
}
void add_timer(timer_event_t *ev, msec_uint_t msec)
{
    msec_uint_t key;
    msec_int_t diff;
    key = current_msec + msec;
    diff = (msec_int_t)(key - ev->timer.key);
    if(abs(diff) < TIMER_LAZY_DELAY)
    {
        return;
    }
    //del_timer(ev);
    ev->timer.key = key;
    rbtree_insert(&timer_rbtree, &ev->timer);
}

rbtree_node_t tq_timer_new(timer_event_t *arg,void (*func)(timer_event_t *ev),int s,char *dbg_info)
{
	timer_event_t *tev = (timer_event_t *)malloc(sizeof(timer_event_t));
	if(tev == NULL)
	{
		perror("malloc");
		exit(1);
	}
	memset(tev, 0, sizeof(timer_event_t));
	if ( arg != NULL) {
		memcpy(tev,arg,sizeof(timer_event_t));
		//debug_info("memcpy timer data [%s] end",tev->data);
	}
	time_update();
	tev->timer.key = current_msec;
	tev->handler = func;
	add_timer(tev,s);
	//debug_info("timer info %s",dbg_info);
	return tev->timer;
}
void free_all_timer()
{
	timer_event_t *ev;
	rbtree_node_t *node, *root, *sentinel;
    sentinel = timer_rbtree.sentinel;
    while(1)
    {
		root = timer_rbtree.root;
		if(root == sentinel)
		{
			return;
		}
		node = rbtree_min(root, sentinel);
		ev = (timer_event_t*)((char*)node - offsetof(timer_event_t, timer));
		rbtree_delete(&timer_rbtree,&ev->timer);
		free(ev);
		ev = NULL;
    }
}