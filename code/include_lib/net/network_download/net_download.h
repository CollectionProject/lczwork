//----------------------------------------------------------------------------//
/**
 ******************************************************************************
 * @file    net_download.h
 * @author
 * @version
 * @brief   This file provides the api of net audio download.
 ******************************************************************************
 * @attention
 *
 * Copyright(c) 2017, ZhuHai JieLi Technology Co.Ltd. All rights reserved.
 ******************************************************************************
 */

#ifndef NET_DOWNLOAD_H
#define NET_DOWNLOAD_H

#include "typedef.h"

struct net_download_parm {
    char *url;
    u32 seek_low_range;
    u32 seek_high_range;
    u32 cbuf_size;
    u32 timeout_millsec;	//socket的连接和读数的超时
    u32 read_timeout;		//解码器读网络buf的超时(ms)
    u8 prio;
    u8 max_reconnect_cnt;
};

enum {
    AI_SPEAK_PRIO,
    AI_ALARM_PRIO,
    AI_MEDIA_PRIO,
};

int net_download_open(void **priv, struct net_download_parm *parm);
int net_download_read(void *priv, void *buf, u32 len);
int net_download_seek(void *priv, u32 offset, int orig);
int net_download_close(void *priv);
int net_download_check_ready(void *priv);
char *net_download_get_media_type(void *priv);
int net_download_get_file_len(void *priv);
int net_download_set_pp(void *priv, u8 pp);
int net_download_set_read_timeout(void *priv, u32 timeout_ms);
void net_download_buf_inactive(void *priv);

#endif

