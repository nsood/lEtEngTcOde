#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/inet.h>

#include "spp_config.h"
#include "spp_network.h"
#include "spp_timer.h"
#include "spp_comm.h"

#if 0
static const char *sips[] = {
	"39.108.167.249",
	"61.151.186.121",
	"61.151.186.122",
	"61.151.186.123",
	"61.151.186.124",
	"101.226.68.98",
	"101.226.212.35",
	"101.226.113.204",
	"101.227.139.162",
	"101.227.139.219",
	"101.227.143.68",
	"101.227.162.12",
	"101.227.162.13",
	"180.163.26.37",
	"180.163.26.38",
	"180.163.26.119",
	"180.163.26.123",
	"180.163.26.124",
};

static const char *cips[] = {
	"192.168.11.4",
	"192.168.11.5",
	"192.168.11.6",
	"192.168.11.7",
	"192.168.11.8",
	"192.168.11.9",
	"192.168.11.3",
	"192.168.11.2",
};
#endif

static int __init spp_init(void)
{	
	
	int err = spp_config_init();
	if (err) 
		goto conf_err;
	
	printk("spp config init done\n");
#if 0
	u32 i;
	for (i = 0 ; i < sizeof(sips)/sizeof(sips[0]) ; i++) 
		spp_sip_add(in_aton(sips[i]));

	for (i = 0 ; i < sizeof(cips)/sizeof(cips[0]) ; i++) 
		spp_cip_add(in_aton(cips[i]));
#endif
    err = spp_comm_init();
	if (err)
		goto comm_err;
	printk("spp comm init done\n");

#ifdef _USE_SPP_TIMER
	err = spp_timer_init();
	if (err) 
		goto timer_err;
	printk("spp timer init done\n");
#endif

	err = spp_network_init();
	if (err)
		goto network_err;

	return err;
	
network_err:
	
#ifdef _USE_SPP_TIMER	
	spp_timer_fini();
timer_err:
#endif

	spp_comm_fini();
comm_err:
	spp_config_fini();
conf_err:
	return err;
}

static void __exit spp_fini(void)
{
	spp_network_stop();

#ifdef _USE_SPP_TIMER	
	spp_timer_fini();
#endif

	spp_network_fini();
    spp_comm_fini();
	spp_config_fini();
	
	printk("spp fini done\n");
}

module_init(spp_init);
module_exit(spp_fini);
MODULE_AUTHOR("ZhouYiding");
MODULE_LICENSE("GPL");
