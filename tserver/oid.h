/* vi: set sw=4 ts=4 sts=4: */
/*
 *	utils.c -- System Utilities 
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/goahead/src/oid.h#1 $
 */

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE				0x8BE0
#endif
#define SIOCIWFIRSTPRIV				SIOCDEVPRIVATE
#endif

#ifndef AP_MODE
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x0E)
#else
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x01)
#endif	/* AP_MODE */

#define RTPRIV_IOCTL_SET			(SIOCIWFIRSTPRIV + 0x02)

#ifdef DBG
#define RTPRIV_IOCTL_BBP			(SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC			(SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P			(SIOCIWFIRSTPRIV + 0x07)
#endif

#define RTPRIV_IOCTL_STATISTICS				(SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE		(SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA			(SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY			(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY			(SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE			(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_STATIC_WEP_COPY		(SIOCIWFIRSTPRIV + 0x10)
#define RTPRIV_IOCTL_WSC_PROFILE			(SIOCIWFIRSTPRIV + 0x12)
#define RT_QUERY_ATE_TXDONE_COUNT			0x0401
#define OID_GET_SET_TOGGLE					0x8000

/* JCG wangxg++: for STA statistics count info */
#define RTPRIV_IOCTL_GSTACOUNTINFO			(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GAPCLIENTINFO			(SIOCIWFIRSTPRIV + 0x1D) /* FIXME: why can not use 0x1C?? */

#define OID_802_11_NETWORK_TYPES_SUPPORTED          0x0103
#define OID_802_11_NETWORK_TYPE_IN_USE              0x0104
#define OID_802_11_RSSI_TRIGGER                     0x0107
#define RT_OID_802_11_RSSI							0x0108 //rt2860 only , kathy
#define RT_OID_802_11_RSSI_1						0x0109 //rt2860 only , kathy
#define RT_OID_802_11_RSSI_2						0x010A //rt2860 only , kathy
#define OID_802_11_NUMBER_OF_ANTENNAS               0x010B
#define OID_802_11_RX_ANTENNA_SELECTED              0x010C
#define OID_802_11_TX_ANTENNA_SELECTED              0x010D
#define OID_802_11_SUPPORTED_RATES                  0x010E
#define OID_802_11_ADD_WEP                          0x0112
#define OID_802_11_REMOVE_WEP                       0x0113
#define OID_802_11_DISASSOCIATE                     0x0114
#define OID_802_11_PRIVACY_FILTER                   0x0118
#define OID_802_11_ASSOCIATION_INFORMATION          0x011E
#define OID_802_11_TEST                             0x011F
#define RT_OID_802_11_COUNTRY_REGION                0x0507
#define OID_802_11_BSSID_LIST_SCAN                  0x0508
#define OID_802_11_SSID                             0x0509
#define OID_802_11_BSSID                            0x050A
#define RT_OID_802_11_RADIO                         0x050B
#define RT_OID_802_11_PHY_MODE                      0x050C
#define RT_OID_802_11_STA_CONFIG                    0x050D
#define OID_802_11_DESIRED_RATES                    0x050E
#define RT_OID_802_11_PREAMBLE                      0x050F
#define OID_802_11_WEP_STATUS                       0x0510
#define OID_802_11_AUTHENTICATION_MODE              0x0511
#define OID_802_11_INFRASTRUCTURE_MODE              0x0512
#define RT_OID_802_11_RESET_COUNTERS                0x0513
#define OID_802_11_RTS_THRESHOLD                    0x0514
#define OID_802_11_FRAGMENTATION_THRESHOLD          0x0515
#define OID_802_11_POWER_MODE                       0x0516
#define OID_802_11_TX_POWER_LEVEL                   0x0517
#define RT_OID_802_11_ADD_WPA                       0x0518
#define OID_802_11_REMOVE_KEY                       0x0519
#define OID_802_11_ADD_KEY                          0x0520
#define OID_802_11_CONFIGURATION                    0x0521
#define OID_802_11_TX_PACKET_BURST					0x0522
#define RT_OID_802_11_QUERY_NOISE_LEVEL             0x0523
#define RT_OID_802_11_EXTRA_INFO                    0x0524
#ifdef DBG
#define RT_OID_802_11_HARDWARE_REGISTER             0x0525
#endif
#define OID_802_11_ENCRYPTION_STATUS                OID_802_11_WEP_STATUS
#define OID_802_11_ACL_LIST                         0x052A

#define RT_OID_DEVICE_NAME                          0x0607
#define RT_OID_VERSION_INFO                         0x0608
#define OID_802_11_BSSID_LIST                       0x0609
#define OID_802_3_CURRENT_ADDRESS                   0x060A
#define OID_GEN_MEDIA_CONNECT_STATUS                0x060B
#define RT_OID_802_11_QUERY_LINK_STATUS             0x060C
#define OID_802_11_RSSI                             0x060D
#define OID_802_11_STATISTICS                       0x060E
#define OID_GEN_RCV_OK                              0x060F
#define OID_GEN_RCV_NO_BUFFER                       0x0610
#define RT_OID_802_11_QUERY_EEPROM_VERSION          0x0611
#define RT_OID_802_11_QUERY_FIRMWARE_VERSION        0x0612
#define RT_OID_802_11_QUERY_LAST_RX_RATE            0x0613
#define RT_OID_802_11_TX_POWER_LEVEL_1              0x0614
#define RT_OID_802_11_QUERY_PIDVID                  0x0615

//#if WPA_SUPPLICANT_SUPPORT
#define OID_SET_COUNTERMEASURES                     0x0616
#define OID_802_11_SET_IEEE8021X                    0x0617
#define OID_802_11_SET_IEEE8021X_REQUIRE_KEY        0x0618
#define OID_802_11_PMKID                            0x0620
#define RT_OID_WPA_SUPPLICANT_SUPPORT					0x0621
#define RT_OID_WE_VERSION_COMPILED                  0x0622
//#endif

//rt2860 , kathy
#define	RT_OID_802_11_SNR_0							0x0630
#define	RT_OID_802_11_SNR_1							0x0631
#define	RT_OID_802_11_QUERY_LAST_TX_RATE			0x0632
#define	RT_OID_802_11_QUERY_HT_PHYMODE				0x0633
#define	RT_OID_802_11_SET_HT_PHYMODE				0x0634
#define	OID_802_11_RELOAD_DEFAULTS					0x0635
#define	RT_OID_802_11_QUERY_APSD_SETTING			0x0636
#define	RT_OID_802_11_SET_APSD_SETTING				0x0637
#define	RT_OID_802_11_QUERY_APSD_PSM				0x0638
#define	RT_OID_802_11_SET_APSD_PSM					0x0639
#define	RT_OID_802_11_QUERY_DLS						0x063A
#define	RT_OID_802_11_SET_DLS						0x063B
#define	RT_OID_802_11_QUERY_DLS_PARAM				0x063C
#define	RT_OID_802_11_SET_DLS_PARAM					0x063D
#define RT_OID_802_11_QUERY_WMM              		0x063E
#define RT_OID_802_11_SET_WMM      					0x063F
#define RT_OID_802_11_QUERY_IMME_BA_CAP				0x0640
#define RT_OID_802_11_SET_IMME_BA_CAP				0x0641
#define RT_OID_802_11_QUERY_BATABLE					0x0642
#define RT_OID_802_11_ADD_IMME_BA					0x0643
#define RT_OID_802_11_TEAR_IMME_BA					0x0644
#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_802_11_QUERY_DAT_HT_PHYMODE          0x0646
#define RT_OID_QUERY_MULTIPLE_CARD_SUPPORT          0x0647
#define OID_802_11_SET_PSPXLINK_MODE				0x0648
#define OID_802_11_SET_PASSPHRASE					0x0649
#if 1//def CONFIG_RT2860V2_AP_V24_DATA_STRUCTURE
#define RT_OID_802_11_SNR_2							0x067A
#define RT_OID_802_11_STREAM_SNR					0x067b
#define RT_OID_802_11_QUERY_TXBF_TABLE				0x067C
#else
#define RT_OID_802_11_SNR_2							0x064a
#define RT_OID_802_11_STREAM_SNR					0x064b
#define RT_OID_802_11_QUERY_TXBF_TABLE				0x0650
#endif
#define RT_OID_802_11_WSC_QUERY_PROFILE				0x0750
#define RT_OID_WSC_UUID                             0x0753

// mesh extension OID
#define OID_802_11_MESH_LINK_STATUS             0x0654
#define OID_802_11_MESH_LIST                    0x0655



// Ralink defined OIDs
// Dennis Lee move to platform specific 

#define RT_OID_802_11_BSSID                   (OID_GET_SET_TOGGLE | OID_802_11_BSSID)
#define RT_OID_802_11_SSID                    (OID_GET_SET_TOGGLE | OID_802_11_SSID)
#define RT_OID_802_11_INFRASTRUCTURE_MODE     (OID_GET_SET_TOGGLE | OID_802_11_INFRASTRUCTURE_MODE)
#define RT_OID_802_11_ADD_WEP                 (OID_GET_SET_TOGGLE | OID_802_11_ADD_WEP)
#define RT_OID_802_11_ADD_KEY                 (OID_GET_SET_TOGGLE | OID_802_11_ADD_KEY)
#define RT_OID_802_11_REMOVE_WEP              (OID_GET_SET_TOGGLE | OID_802_11_REMOVE_WEP)
#define RT_OID_802_11_REMOVE_KEY              (OID_GET_SET_TOGGLE | OID_802_11_REMOVE_KEY)
#define RT_OID_802_11_DISASSOCIATE            (OID_GET_SET_TOGGLE | OID_802_11_DISASSOCIATE)
#define RT_OID_802_11_AUTHENTICATION_MODE     (OID_GET_SET_TOGGLE | OID_802_11_AUTHENTICATION_MODE)
#define RT_OID_802_11_PRIVACY_FILTER          (OID_GET_SET_TOGGLE | OID_802_11_PRIVACY_FILTER)
#define RT_OID_802_11_BSSID_LIST_SCAN         (OID_GET_SET_TOGGLE | OID_802_11_BSSID_LIST_SCAN)
#define RT_OID_802_11_WEP_STATUS              (OID_GET_SET_TOGGLE | OID_802_11_WEP_STATUS)
#define RT_OID_802_11_RELOAD_DEFAULTS         (OID_GET_SET_TOGGLE | OID_802_11_RELOAD_DEFAULTS)
#define RT_OID_802_11_NETWORK_TYPE_IN_USE     (OID_GET_SET_TOGGLE | OID_802_11_NETWORK_TYPE_IN_USE)
#define RT_OID_802_11_TX_POWER_LEVEL          (OID_GET_SET_TOGGLE | OID_802_11_TX_POWER_LEVEL)
#define RT_OID_802_11_RSSI_TRIGGER            (OID_GET_SET_TOGGLE | OID_802_11_RSSI_TRIGGER)
#define RT_OID_802_11_FRAGMENTATION_THRESHOLD (OID_GET_SET_TOGGLE | OID_802_11_FRAGMENTATION_THRESHOLD)
#define RT_OID_802_11_RTS_THRESHOLD           (OID_GET_SET_TOGGLE | OID_802_11_RTS_THRESHOLD)
#define RT_OID_802_11_RX_ANTENNA_SELECTED     (OID_GET_SET_TOGGLE | OID_802_11_RX_ANTENNA_SELECTED)
#define RT_OID_802_11_TX_ANTENNA_SELECTED     (OID_GET_SET_TOGGLE | OID_802_11_TX_ANTENNA_SELECTED)
#define RT_OID_802_11_SUPPORTED_RATES         (OID_GET_SET_TOGGLE | OID_802_11_SUPPORTED_RATES)
#define RT_OID_802_11_DESIRED_RATES           (OID_GET_SET_TOGGLE | OID_802_11_DESIRED_RATES)
#define RT_OID_802_11_CONFIGURATION           (OID_GET_SET_TOGGLE | OID_802_11_CONFIGURATION)
#define RT_OID_802_11_POWER_MODE              (OID_GET_SET_TOGGLE | OID_802_11_POWER_MODE)


/* for WPS --YY  */
#define RT_OID_SYNC_RT61                            0x0D010750
#define RT_OID_WSC_QUERY_STATUS                     ((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE							((RT_OID_SYNC_RT61 + 0x02) & 0xffff)

typedef union _MACHTTRANSMIT_SETTING {
	struct  {
		unsigned short  MCS:7;  // MCS
		unsigned short  BW:1;   //channel bandwidth 20MHz or 40 MHz
		unsigned short  ShortGI:1;
		unsigned short  STBC:2; //SPACE
		unsigned short	eTxBF:1;
		unsigned short	rsv:1;
		unsigned short	iTxBF:1;
		unsigned short  MODE:2; // Use definition MODE_xxx.
	} field;
	unsigned short      word;
} MACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char			ApIdx;
	unsigned char           Addr[6];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int			LastRxRate;
	int						StreamSnr[3];
	int						SoundingRespSnr[3];
#if 0
	short					TxPER;
	short					reserved;
#endif
#if defined (RT2860_VOW_SUPPORT)
	char					Tx_Per;
#endif
} RT_802_11_MAC_ENTRY;

// RT2860V2 for RT305x/RT3883/RT5350/MT7620
#if defined(CONFIG_RT2860V2_AP) || defined(CONFIG_RT2860V2_AP_MODULE)
#if defined(CONFIG_FIRST_IF_RT2860)
#if defined(CONFIG_RALINK_RT3052)
#define MAX_NUMBER_OF_MAC               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#elif defined(CONFIG_RALINK_RT3883)
#define MAX_NUMBER_OF_MAC               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#elif defined(CONFIG_RALINK_RT5350)
#define MAX_NUMBER_OF_MAC               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#elif defined(CONFIG_RALINK_MT7620)
#define MAX_NUMBER_OF_MAC               64 // JCG wangxg: 32->64 if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#endif
#define RT2860_EEPROM_SIZE 	512
#endif /* CONFIG_FIRST_IF_RT2860 */
#endif /* CONFIG_RT2860V2_AP */

// RLT_WIFI for MT7612/MT7602/MT7620
#if defined(CONFIG_RLT_WIFI) || defined(CONFIG_RLT_WIFI_MODULE)
#if defined (CONFIG_FIRST_IF_MT7602E) || defined (CONFIG_FIRST_IF_MT7612E) || defined (CONFIG_FIRST_IF_MT7620)
#if defined(CONFIG_MAC_REPEATER_SUPPORT)
#define MAX_NUMBER_OF_MAC               (116 - (16 + 1) * 1)
#else
#define MAX_NUMBER_OF_MAC               116
#endif /* CONFIG_MAC_REPEATER_SUPPORT */
#define RT2860_EEPROM_SIZE 	512
#endif /* CONFIG_FIRST_IF_MT7602E || CONFIG_FIRST_IF_MT7612E || CONFIG_FIRST_IF_MT7620 */
#endif /* CONFIG_RLT_WIFI */

// MT_RLT_WIFI for MT7603
#if defined(CONFIG_MT_RLT_WIFI) || defined(CONFIG_MT_RLT_WIFI_MODULE)
#if defined (CONFIG_FIRST_IF_MT7603E)
#ifdef CONFIG_MT_RLT_MAC_REPEATER_SUPPORT
#define MAX_NUMBER_OF_MAC				(75 - (16 + 1) * 1)
#else
#define MAX_NUMBER_OF_MAC            	75
#endif /* CONFIG_MT_RLT_MAC_REPEATER_SUPPORT */
#define RT2860_EEPROM_SIZE 	512
#endif /* CONFIG_FIRST_IF_MT7603E */
#endif

// XT_MT_WIFI for MT7615
#if defined(CONFIG_XT_MT_WIFI) || defined(CONFIG_XT_MT_WIFI_MODULE)
#if defined(CONFIG_FIRST_IF_MT7615E)
#define MAX_NUMBER_OF_MAC               75
#define RT2860_EEPROM_SIZE 	1024
#endif
#endif

// MT_WIFI for MT7628
#if defined(CONFIG_MT_WIFI) || defined(CONFIG_MT_WIFI_MODULE)
#if defined (CONFIG_FIRST_IF_MT7628)
#ifdef CONFIG_MT_MAC_REPEATER_SUPPORT
#ifdef CONFIG_MT_APCLI_CONNECTION_TRIAL
#define MAX_NUMBER_OF_MAC				(75 - (24 + 1) * 2)
#else
#define MAX_NUMBER_OF_MAC				(75 - (24 + 1) * 1)
#endif /* CONFIG_MT_APCLI_CONNECTION_TRIAL */
#else
#define MAX_NUMBER_OF_MAC            	75
#endif /* CONFIG_MT_MAC_REPEATER_SUPPORT */
#define RT2860_EEPROM_SIZE 	512
#endif /* CONFIG_FIRST_IF_MT7628 */
#endif /* CONFIG_MT_WIFI */

// Others...
#ifndef MAX_NUMBER_OF_MAC
#define MAX_NUMBER_OF_MAC               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#define RT2860_EEPROM_SIZE 	512
#endif

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_MAC_ENTRY      Entry[MAX_NUMBER_OF_MAC]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;

typedef struct _RT_802_11_MAC_ENTRY_RTDEV {
	unsigned char			ApIdx;
	unsigned char           Addr[6];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int			LastRxRate;
	int						StreamSnr[3];
	int						SoundingRespSnr[3];
#if 0
	short					TxPER;
	short					reserved;
#endif
#if defined (RTDEV_VOW_SUPPORT)
	char					Tx_Per;
#endif
} RT_802_11_MAC_ENTRY_RTDEV;

// RTPCI for RT309x
#if defined(CONFIG_RTPCI_AP) || defined(CONFIG_RTPCI_AP_MODULE)
#if defined(CONFIG_SECOND_IF_RTPCI)
#define MAX_NUMBER_OF_MAC_RTDEV               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#define RTDEV_EEPROM_SIZE 	512
#endif
#endif /* CONFIG_RTPCI_AP */

// MT7610_AP for MT7610E
#if defined(CONFIG_MT7610_AP) || defined(CONFIG_MT7610_AP_MODULE)
#if defined(CONFIG_SECOND_IF_MT7610E)
#define MAX_NUMBER_OF_MAC_RTDEV               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#define RTDEV_EEPROM_SIZE 	512
#endif
#endif

// RLT_WIFI for MT7612/MT7602
#if defined(CONFIG_RLT_WIFI) || defined(CONFIG_RLT_WIFI_MODULE)
#if defined (CONFIG_SECOND_IF_MT7602E) || defined (CONFIG_SECOND_IF_MT7612E)
#if defined(CONFIG_MAC_REPEATER_SUPPORT)
#define MAX_NUMBER_OF_MAC_RTDEV               (116 - (16 + 1) * 1)
#else
#define MAX_NUMBER_OF_MAC_RTDEV               116
#endif /* CONFIG_MAC_REPEATER_SUPPORT */
#define RTDEV_EEPROM_SIZE 	512
#endif /* CONFIG_SECOND_IF_MT7602E || CONFIG_SECOND_IF_MT7612E */
#endif /* CONFIG_RLT_WIFI */

// XT_MT_WIFI for MT7615
#if defined(CONFIG_XT_MT_WIFI) || defined(CONFIG_XT_MT_WIFI_MODULE)
#if defined(CONFIG_SECOND_IF_MT7615E) || defined(CONFIG_XT_MT_DBDC_MODE)
#define MAX_NUMBER_OF_MAC_RTDEV               75
#define RTDEV_EEPROM_SIZE 	1024
#endif
#endif

// Others...
#ifndef MAX_NUMBER_OF_MAC_RTDEV
#define MAX_NUMBER_OF_MAC_RTDEV               32 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#define RTDEV_EEPROM_SIZE 	512
#endif

typedef struct _RT_802_11_MAC_TABLE_RTDEV {
	unsigned long            Num;
	RT_802_11_MAC_ENTRY_RTDEV      Entry[MAX_NUMBER_OF_MAC_RTDEV]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE_RTDEV;

#define MAX_NUMBER_OF_TXBF                     MAX_NUMBER_OF_MAC
#define MAX_NUMBER_OF_TXBF_RTDEV               MAX_NUMBER_OF_MAC_RTDEV

// RT_OID_802_11_QUERY_TXBF_TABLE results
typedef struct {
	unsigned long	TxSuccessCount;
	unsigned long	TxRetryCount;
	unsigned long	TxFailCount;
	unsigned long	ETxSuccessCount;
	unsigned long	ETxRetryCount;
	unsigned long	ETxFailCount;
	unsigned long	ITxSuccessCount;
	unsigned long	ITxRetryCount;
	unsigned long	ITxFailCount;
} RT_COUNTER_TXBF;
  
typedef struct {
	unsigned long           Num;
	RT_COUNTER_TXBF         Entry[MAX_NUMBER_OF_TXBF];
} RT_802_11_TXBF_TABLE;

typedef struct {
	unsigned long           Num;
	RT_COUNTER_TXBF         Entry[MAX_NUMBER_OF_TXBF_RTDEV];
} RT_802_11_TXBF_TABLE_RTDEV;

#define MAX_NUMBER_OF_ACL	64
typedef struct _RT_802_11_ACL_ENTRY {
	unsigned char Addr[6];
	unsigned short Rsv;
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

typedef struct _RT_802_11_ACL {
	unsigned long Policy;           /* 0-disable, 1-positive list, 2-negative list */
	unsigned long Num;
	RT_802_11_ACL_ENTRY Entry[MAX_NUMBER_OF_ACL];
} RT_802_11_ACL, *PRT_802_11_ACL;

