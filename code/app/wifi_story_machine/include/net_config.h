#ifndef _NET_CONFIG_H__
#define _NET_CONFIG_H__

/*------------网络配置----------*/
#define ACCESS_NUM 1
#define UUID "f2dd3cd7-b026-40aa-aaf4-f6ea07376490"
/*#define CONFIG_ENABLE_VLIST                          */
#define WIFI_CAM_PREFIX    "wifi_camera_ac54_"
//#define WIFI_CAM_SUFFIX     "xxxxxxxxxxxxx"
#define WIFI_CAM_WIFI_PWD    "12345678"
#define IPERF_ENABLE
//#define CONFIG_FORCE_RESET_VM  //檫除vm所有配置


#define CONFIG_RTS_JPEG_ENABLE
//#define CONFIG_RTS_H264_ENABLE
/*#define CONFIG_NET_TCP_ENABLE*/
#define CONFIG_NET_UDP_ENABLE


/*两者只能选一*/
//#define CONFIG_NET_CLIENT  //wifi投屏 发送端
//#define CONFIG_NET_SERVER  //wifi投屏 接收端
/*------------手机投影 开关----------*/
#define APP_PROJECTION   0

/*-------------网络端口----------------*/

#define CTP_CTRL_PORT   3333
#define CDP_CTRL_PORT   2228
#define PROJECTION_PORT 2230
#define VIDEO_PREVIEW_PORT 2226
#define VIDEO_PLAYBACK_PORT 2223
#define HTTP_PORT           8080
#define RTSP_PORT           554




//视频库内使用
#define _DUMP_PORT          2229
#define _FORWARD_PORT    2224
#define _BEHIND_PORT     2225


#endif

