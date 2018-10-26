#ifndef APP_CONFIG_H
#define APP_CONFIG_H





//#define CONFIG_BOARD_5213B_STORY
 #define  CONFIG_BOARD_GR202_2M



#ifdef CONFIG_BOARD_5213B_STORY
#define __CPU_AC521x__
#define __FLASH_SIZE__    (4 * 1024 * 1024)
#define CONFIG_SFC_ENABLE
#ifdef CONFIG_SFC_ENABLE
#define __SDRAM_SIZE__    (2 * 1024 * 1024)
#else
#define __SDRAM_SIZE__    (8 * 1024 * 1024)
#endif
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
#define CONFIG_NET_ENABLE 						/* 网络配置 */
#endif

#ifdef CONFIG_BOARD_GR202_2M
#define __CPU_AC521x__
#define __FLASH_SIZE__    (4 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)
#define CONFIG_SFC_ENABLE
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
#define CONFIG_NET_ENABLE 						/* 网络配置 */
#endif

//#define CONFIG_DATABASE_2_RTC                   /* 系统配置存RTC */
#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
//#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
//#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#define CONFIG_ADKEY_ENABLE                     /* AD按键开关  */
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
//#define CONFIG_DISPLAY_ENABLE                   /* 摄像头显示使能 */
#define CONFIG_MP3_DEC_ENABLE
#define CONFIG_WMA_DEC_ENABLE
#define CONFIG_M4A_DEC_ENABLE
#define CONFIG_WAV_DEC_ENABLE
#define CONFIG_WAV_ENC_ENABLE
#define CONFIG_AMR_DEC_ENABLE
#define CONFIG_AMR_ENC_ENABLE
// #define CONFIG_AAC_DEC_ENABLE
#define CONFIG_SPEEX_ENC_ENABLE

#ifdef CONFIG_NET_ENABLE
#define CONFIG_WIFI_ENABLE  					/* 无线WIFI */
 #define CONFIG_PROFILE_UPDATE					//每次开机都更新一次profile
// #define CONFIG_WECHAT_SDK_ENABLE             //使用杰理公众号
#define CONFIG_TURING_SDK_ENABLE             //使用图灵公众号
// #define CONFIG_DEEPBRAIN_SDK_ENABLE
//#define CONFIG_DUER_SDK_ENABLE
//#define CONFIG_DUER_WECHAT_ENABLE
#endif


#ifdef CONFIG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#ifdef CONFIG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifdef CONFIG_SD2_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd2"
#define SDX_DEV					"sd2"
#endif

#define CONFIG_ROOT_PATH     	    CONFIG_STORAGE_PATH"/C/"

#define CONFIG_MUSIC_PATH_SD        CONFIG_ROOT_PATH

#define CONFIG_MUSIC_PATH_FLASH             "mnt/spiflash/C/"
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/spiflash/audlogo/"


#define RTOS_STACK_CHECK_ENABLE //是否启用定时检查任务栈


/*升级版本号*/
/*
版本    说明
主版本  用于重大升级，例如整体架构，接口都发生重大改变，导致版本不兼容，此时应该升级主版本
次版本  用于功能增加及升级，整体架构不变，接口完全兼容。次版本必须往下兼容，即1.5.x 必须兼容1.4.x、1.3.x、1.2.x、1.1.x、1.0.x
修订版本    用于内部发布及BUG修复
*/

#define OTA_MAJOR   0
#define OTA_MINOR   2
#define OTA_PATCH   0




#ifndef __LD__
#include "cpu_config.h"
#include "board.h"
#endif































#endif

