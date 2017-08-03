#ifndef _SPP_CONFIG_H
#define _SPP_CONFIG_H

int spp_config_init(void);
void spp_config_fini(void);

bool spp_sip_match(u32 ip,bool del);
bool spp_cip_match(u32 ip,bool del);
struct net_device *spp_lan(void);

/*以下为对外接口*/

/*添加server/client ip*/
int spp_sip_add(u32 ip);
void spp_sip_del(u32 ip);
int spp_cip_add(u32 ip);
void spp_cip_del(u32 ip);

/*开启/关闭wmm特性*/
bool spp_wmm_get(void);
void spp_wmm_set(bool);

/*设置/获取多发时间间隔*/
u16 spp_dup_get(u16 index);
/*index 第几次 val为间隔值*/
void spp_dup_set(u16 index,u16 val);

/*设置去重有效时间*/
u16 spp_trim_get(void);
void spp_trim_set(u16 v);



#endif
