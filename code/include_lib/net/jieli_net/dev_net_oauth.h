
#ifndef __DEV_NET_OAUTH_H
#define __DEV_NET_OAUTH_H

#include "typedef.h"


void dev_net_oauth_set_user(char *username, char *password, char *dev_mac);

int jieliapp_chat_send_voice(char *filename);
int jieliapp_chat_send_voice_buffer(char *buffer, u32 len);

char *dev_net_get_access_token(void);

// if type==NULL
//     no voice
typedef void (*jl_chat_voice)(char *type, char *buf);
int jieliapp_chat_voice_set_read(void);
int jieliapp_chat_get_new_voice(jl_chat_voice cb);
int jieliapp_chat_get_one_voice(jl_chat_voice cb);
int jieliapp_chat_get_next_voice(jl_chat_voice cb);
int jieliapp_chat_get_prev_voice(jl_chat_voice cb);

int jieli_get_music_url(const char *str, u32 album_id, u32 meta_sn, u8 *data, u32 data_len);


#endif

