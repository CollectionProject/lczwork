
#ifndef _VIDEO_MUSIC_
#define _VIDEO_MUSIC_

#include "system/includes.h"
#include "app_config.h"
#include "server/audio_server.h"

struct music_dec_ops {
    int (*switch_dir)(int fsel_mode);
    int (*switch_file)(int fsel_mode);
    int (*dec_file)(void *file, int, void *, int);
    int (*dec_play_pause)(u8 notify);
    int (*dec_breakpoint)(int);
    int (*dec_stop)(int);
    int (*dec_volume)(int);
    int (*dec_progress)(int);
    int (*exit)();
};

struct net_breakpoint {
    u32 fptr;
    u16 url_len;
    char *url;
    const char *ai_name;
    struct audio_dec_breakpoint dec_bp;
};

struct app_music_hdl {
#ifdef CONFIG_NET_ENABLE
    u8 wifi_config_state;
    u8 wifi_connected;
    u8 ai_connected;
    u8 wechat_connected;
    u8 listening;
    char wechat_speech_url[512];
    u8 wechat_speech_read_flag;
    u16 wait_download;
    u16 wechat_state;
    int download_ready;
    void *net_file;
    void *rec_file;
    const char *url;
    const char *ai_name;
    struct server *wechat_server;
    struct server *enc_server;
    struct server *ai_server;
    struct net_breakpoint net_bp;
#endif
    int play_time;
    char volume;
    u8 get_breakpoint;
    u8 play_voice_prompt;
    int low_power;
    u32 bp_file_sclust;
    int wait_switch_file;
    FILE *local_bp_file;
    FILE *file;
    int dec_end_args[4];
    void *dec_end_file;
    void *dec_end_handler;
    struct vfscan *fscan;
    struct vfscan *dir_list;
    struct server *dec_server;
    struct audio_dec_breakpoint local_bp;
    const char *local_path;
    const struct music_dec_ops *dec_ops;
    struct server *led_ui;
};





#endif

