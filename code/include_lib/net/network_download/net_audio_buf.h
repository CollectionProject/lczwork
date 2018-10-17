//----------------------------------------------------------------------------//
/**
 ******************************************************************************
 * @file    net_audio_buf.h
 * @author
 * @version
 * @brief   This file provides the api of net audio buffer.
 ******************************************************************************
 * @attention
 *
 * Copyright(c) 2017, ZhuHai JieLi Technology Co.Ltd. All rights reserved.
 ******************************************************************************
 */

#ifndef NET_AUDIO_BUF_H
#define NET_AUDIO_BUF_H

#include "circular_buf.h"

/*! \addtogroup NET_AUDIO_BUF
 *  @ingroup COMMON
 *  @brief	Net audio buf api
 *  @{
 */


/*----------------------------------------------------------------------------*/
/**@brief  初始化网络音频buf
   @param  cbuf_size: buffer的字节长度
   @return 指向net_buf_t结构体的指针
   @return NULL: 初始化失败
   @note
*/
/*----------------------------------------------------------------------------*/
void *net_buf_init(u32 cbuf_size);

/*----------------------------------------------------------------------------*/
/**@brief  释放网络音频buf
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_uninit(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  激活网络音频buf
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_active(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  失活网络音频buf
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_inactive(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  设置网络音频buf的超时时间
   @param  timeout_ms: 超时时间，以ms为单位
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_set_time_out(u32 timeout_ms, void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  设置网络音频buf的结束标志
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_set_file_end(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  设置网络音频buf的暂停标志
   @param  pp: 0-继续，1-暂停
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_set_pp(u8 pp, void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  开始seek网络音频buf
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_seek_start(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  停止seek网络音频buf
   @param  hdl: 指向net_buf_t结构体的指针
   @note
*/
/*----------------------------------------------------------------------------*/
void net_buf_seek_stop(void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  写入网络音频buf
   @param[in]  w_buf: 需要写入的数据
   @param  len: 需要写入的数据的字节长度
   @param  hdl: 指向net_buf_t结构体的指针
   @return 成功写入的长度
   @note
*/
/*----------------------------------------------------------------------------*/
u32 net_buf_write(u8 *w_buf, int len, void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  读取网络音频buf
   @param[out]  r_buf: 保存读取到的数据
   @param  len: 需要读取的数据的字节长度
   @param  hdl: 指向net_buf_t结构体的指针
   @return 成功读取的长度
   @note	返回-1为主动退出，返回-1为超时退出
*/
/*----------------------------------------------------------------------------*/
int net_buf_read(void *r_buf, int len, void *hdl);

/*----------------------------------------------------------------------------*/
/**@brief  seek网络音频buf
   @param  offset: 偏移量
   @param  orig: 起止位置
   @param  hdl: The point of private data
   @note
*/
/*----------------------------------------------------------------------------*/
int net_buf_seek(u32 offset, int orig, void *hdl);

/*! @}*/

#endif
