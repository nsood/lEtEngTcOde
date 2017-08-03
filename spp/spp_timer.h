#ifndef _SPP_TIMER_H
#define _SPP_TIMER_H
#ifdef _USE_SPP_TIMER
struct spp_timer;

int  spp_timer_init(void);
void  spp_timer_fini(void);
bool spp_timer_new(void (*func)(void *),void *v,void (*cancel)(void *),unsigned long delay);
#endif

#endif


