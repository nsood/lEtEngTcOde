#ifndef _SPP_CONFIG_H
#define _SPP_CONFIG_H

int spp_config_init(void);
void spp_config_fini(void);

bool spp_sip_match(u32 ip,bool del);
bool spp_cip_match(u32 ip,bool del);
struct net_device *spp_lan(void);

/*����Ϊ����ӿ�*/

/*���server/client ip*/
int spp_sip_add(u32 ip);
void spp_sip_del(u32 ip);
int spp_cip_add(u32 ip);
void spp_cip_del(u32 ip);

/*����/�ر�wmm����*/
bool spp_wmm_get(void);
void spp_wmm_set(bool);

/*����/��ȡ�෢ʱ����*/
u16 spp_dup_get(u16 index);
/*index �ڼ��� valΪ���ֵ*/
void spp_dup_set(u16 index,u16 val);

/*����ȥ����Чʱ��*/
u16 spp_trim_get(void);
void spp_trim_set(u16 v);



#endif
