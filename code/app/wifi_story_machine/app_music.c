#include "app_music.h"
#include "system/includes.h"
#include "server/audio_server.h"
#include "app_config.h"
#include "action.h"
#include "storage_device.h"
#include "key_voice.h"
#include "led_eyes.h"
#include "server/led_ui_server.h"

#ifdef CONFIG_NET_ENABLE
#include "wechat/wechat_server.h"
#include "network_download/net_download.h"
#include "server/ai_server.h"
#endif

enum {
    LISTEN_STATE_STOP,
    LISTEN_STATE_START,
    LISTEN_STATE_RETURN_URL,
};

/* #define CONFIG_STORE_VOLUME */

#define DEC_BUF_LEN      12 * 1024

struct app_music_hdl music_handler;

#define __this 	(&music_handler)


static int app_music_switch_local_device(const char *path);
static int app_music_play_voice_prompt(const char *fname, void *dec_end_handler);

static const struct music_dec_ops local_music_dec_ops;


static const u8 dir_name_chars[][8] = {
    { 0x3F, 0x51, 0x4C, 0x6B, 0x00, 0x00 },   //儿歌
    { 0x45, 0x65, 0x8B, 0x4E, 0x00, 0x00 },   //故事
    { 0xFD, 0x56, 0x66, 0x5B, 0x00, 0x00 },   //国学
    { 0xF1, 0x82, 0xED, 0x8B, 0x00, 0x00 },   //英语
};

static const char *voice_prompt_file[] = {
    "004.mp3",      //儿歌
    "005.mp3",      //故事
    "006.mp3",      //国学
    "007.mp3",      //英语
};

static int app_music_shutdown(int priv)
{
    sys_power_poweroff(0);
    return 0;
}

static void led_ui_post_msg(const char *msg, ...)
{
    union led_uireq req;
    va_list argptr;

    if (__this->play_voice_prompt) {
        return;
    }

    va_start(argptr, msg);

    if (__this->led_ui) {
        req.msg.receiver = "gr202_led";
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->led_ui, LED_UI_REQ_MSG, &req);
    }

    va_end(argptr);

}

static void set_dec_end_handler(void *file, void *handler, int arg0, int arg1)
{
    __this->dec_end_file = file;
    __this->dec_end_args[0] = arg0;
    __this->dec_end_args[1] = arg1;
    __this->dec_end_handler = handler;
}

static void do_dec_end_handler(void *file)
{
    if (file == __this->dec_end_file) {
        __this->dec_end_file = NULL;
        if (__this->dec_end_handler) {
            if (__this->dec_end_args[1] == -1) {
                ((int (*)(int))__this->dec_end_handler)(__this->dec_end_args[0]);
            } else {
                ((int (*)(int, int))__this->dec_end_handler)(__this->dec_end_args[0],
                        __this->dec_end_args[1]);
            }
        }
    }
}

static const char *__get_dirname_file(const char *name, int len)
{
    for (int i = 0; i < ARRAY_SIZE(dir_name_chars); i++) {
        if (!memcmp(dir_name_chars[i], name, len)) {
            return voice_prompt_file[i];
        }
    }
    for (int i = 0; i < ARRAY_SIZE(voice_prompt_file); i++) {
        if (!memcmp(name, voice_prompt_file[i], 3)) {
            return voice_prompt_file[i];
        }
    }

    return NULL;
}

static int __get_dec_breakpoint(struct audio_dec_breakpoint *bp)
{
    int err;
    union audio_req r;

    bp->len = 0;

    r.dec.cmd = AUDIO_DEC_GET_BREAKPOINT;
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (err) {
        return err;
    }

    if (r.dec.status == AUDIO_DEC_START) {
        memcpy(bp, &r.dec.bp, sizeof(*bp));
        put_buf(bp->data, bp->len);
        return 0;
    }
    return -EFAULT;
}

static int __set_dec_volume(int step)
{
    union audio_req req;

    int volume = __this->volume + step;
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    if (volume == __this->volume) {
        return -EINVAL;
    }
    __this->volume = volume;

    log_d("set_dec_volume: %d\n", volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = volume;
    return server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
}

/*
 * 暂停/播放音乐
 */
static int local_music_dec_play_pause(u8 notify)
{
    union audio_req r;

    r.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (r.dec.status == AUDIO_DEC_START) {
        led_ui_post_msg("dec_start");
    } else {
        led_ui_post_msg("dec_pause");
    }

    return 0;
}

static int local_music_dec_stop(int save_breakpoint)
{
    union audio_req req;
    struct vfs_attr attr;

    if (!__this->file) {
        return 0;
    }

    __this->dec_end_handler = NULL;

    puts("local_music_dec_stop\n");

    led_ui_post_msg("dec_stop");
    if (__this->play_voice_prompt) {
        __this->play_voice_prompt = 0;
        sys_key_event_enable();
    }

    if (save_breakpoint) {
        if (0 == __get_dec_breakpoint(&__this->local_bp)) {
            __this->local_bp_file = __this->file;
        }
    }

    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    if (!save_breakpoint) {
        fclose(__this->file);
    }
    __this->file = NULL;

    return 0;
}

/*
 * 选择文件并播放, mode为选择方式，如FSEL_NEXT_FILE
 */
static int local_music_dec_file(void *file, int breakpoint, void *handler, int arg)
{
    int len, err;
    char name[128];
    union audio_req req = {0};

    puts("app_music_dec_file\n");


    if (__this->dec_ops) {
        __this->dec_ops->dec_stop(0);

    }

    set_dec_end_handler(file, handler, arg, -1);

    if (!breakpoint) {
        req.dec.bp.len = 0;
    } else {
        memcpy(&req.dec.bp, &__this->local_bp, sizeof(struct audio_dec_breakpoint));
        __this->local_bp.len = 0;
    }

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = __this->volume;
    req.dec.output_buf      = NULL;
    req.dec.output_buf_len  = DEC_BUF_LEN;
    req.dec.file            = (FILE *)file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = NULL;
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        log_e("audio_dec_open: err = %d\n", err);
        fclose((FILE *)file);
        return err;
    }


    req.dec.cmd = AUDIO_DEC_START;
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        log_e("audio_dec_start: err = %d\n", err);
        fclose((FILE *)file);
        return err;
    }
    __this->file = (FILE *)file;
    __this->dec_ops = &local_music_dec_ops;

    led_ui_post_msg("dec_start");
    puts("play_music_file: suss\n");

    return 0;
}

static int local_music_dec_switch_file(int fsel_mode)
{
    int i = 0;
    FILE *file = NULL;

    if (!__this->fscan) {
        return -ENOENT;
    }

    do {
        file = fselect(__this->fscan, fsel_mode, 0);
        if (file) {
            break;
        }
        if (fsel_mode == FSEL_NEXT_FILE) {
            fsel_mode = FSEL_FIRST_FILE;
        } else if (fsel_mode == FSEL_PREV_FILE) {
            fsel_mode = FSEL_LAST_FILE;
        } else {
            break;
        }
    } while (i++ < __this->fscan->file_number);

    if (!file) {
        return -ENOENT;
    }

    return local_music_dec_file(file, 0, local_music_dec_switch_file, FSEL_LAST_FILE);
}

static int local_music_dec_switch_dir(int fsel_mode)
{
    int len;
    int i = 0;
    char name[64];
    char path[128];
    FILE *dir;
    FILE *file = NULL;

    puts("app_music_dec_switch_dir\n");

    if (!__this->local_path) {
        return app_music_switch_local_device(NULL);
    }

    /*
     * 搜索文件夹
     */
    if (!__this->dir_list) {
        __this->dir_list = fscan(__this->local_path, "-d -sn");
        if (!__this->dir_list || __this->dir_list->file_number == 0) {
            puts("no_music_dir_find\n");
            return -ENOENT;
        }
    }

    /*
     * 选择文件夹
     */
__try:
        do {
            dir = fselect(__this->dir_list, fsel_mode, 0);
            if (dir) {
                i++;
                break;
            }
            if (fsel_mode == FSEL_NEXT_FILE) {
                fsel_mode = FSEL_FIRST_FILE;
            } else if (fsel_mode == FSEL_PREV_FILE) {
                fsel_mode = FSEL_LAST_FILE;
            } else {
                puts("fselect_dir_faild\n");
                return -ENOENT;
            }
        } while (i++ < __this->dir_list->file_number);


    /*
     * 根据文件夹名打开提示音文件
     */
    len = fget_name(dir, (u8 *)name, sizeof(name));
    if (len > 0) {
        const char *note = __get_dirname_file(name, len);
        if (note) {
            fname_to_path(path, CONFIG_VOICE_PROMPT_FILE_PATH, note, strlen(note) + 1);
        } else {
            fname_to_path(path, CONFIG_VOICE_PROMPT_FILE_PATH, name, strlen(name) + 1);
        }
        file = fopen(path, "r");
        if (!file) {
            if (i >= __this->dir_list->file_number) {
                return -ENOENT;
            }
            if (fsel_mode == FSEL_FIRST_FILE) {
                fsel_mode = FSEL_NEXT_FILE;
            } else if (fsel_mode == FSEL_LAST_FILE) {
                fsel_mode = FSEL_PREV_FILE;
            }
            goto __try;
        }
        local_music_dec_file(file, 0, local_music_dec_switch_file, FSEL_FIRST_FILE);
    }

    if (__this->fscan) {
        fscan_release(__this->fscan);
    }

    /*
     * 搜索文件夹下的文件，按序号排序
     */
    fname_to_path(path, __this->local_path, name, len);
    __this->fscan = fscan(path, "-tMP3WMAWAV -sn");

    fclose(dir);

    if (!file) {
        local_music_dec_switch_file(FSEL_FIRST_FILE);
    }

    return 0;
}

static int local_music_dec_breakpoint(int priv)
{
    if (!__this->fscan) {
        return -ENOENT;
    }

    if (__this->local_bp.len == 0) {
        return 0;
    }

    return local_music_dec_file(__this->local_bp_file, 1,
                                local_music_dec_switch_file, FSEL_NEXT_FILE);
}

static int local_musci_dec_progress(int time)
{
    return 0;
}

static int local_music_dec_volume(int step)
{
    return __set_dec_volume(step);
}

static const struct music_dec_ops local_music_dec_ops = {
    .switch_dir     = local_music_dec_switch_dir,
    .switch_file    = local_music_dec_switch_file,
    .dec_file       = local_music_dec_file,
    .dec_breakpoint = local_music_dec_breakpoint,
    .dec_play_pause = local_music_dec_play_pause,
    .dec_volume     = local_music_dec_volume,
    .dec_progress   = local_musci_dec_progress,
    .dec_stop       = local_music_dec_stop,
};

/*
 * flash和sd卡切换
 */
static int app_music_switch_local_device(const char *path)
{
    if (!path) {
        if (storage_device_ready()) {
            path = CONFIG_MUSIC_PATH_SD;
        } else {
            path = CONFIG_MUSIC_PATH_FLASH;
        }
    }

    if (__this->local_path == path) {
        return 0;
    }
    if (__this->dir_list) {
        fscan_release(__this->dir_list);
        __this->dir_list = NULL;
    }

    __this->local_path = path;
    local_music_dec_switch_dir(FSEL_FIRST_FILE);

    return 0;
}


/*
 * ****************************网络播歌部分*************************************
 */
#ifdef CONFIG_NET_ENABLE

static const struct music_dec_ops net_music_dec_ops;

static const struct audio_vfs_ops net_audio_dec_vfs_ops = {
    .fread = net_download_read,
    .fseek = net_download_seek,
    .flen  = net_download_get_file_len,
};

static int __net_download_ready()
{
    __this->download_ready = net_download_check_ready(__this->net_file);
    if (__this->download_ready) {
        return 1;
    }
    return 0;
}

static int net_music_dec_play_pause(u8 notify)
{
    union audio_req r;
    union ai_req req;

    r.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (r.dec.status == AUDIO_DEC_START) {
        /* 播放状态 */
        net_download_set_pp(__this->net_file, 0);
        led_ui_post_msg("dec_start");
    } else if (r.dec.status == AUDIO_DEC_PAUSE) {
        /* 暂停状态 */
        net_download_set_pp(__this->net_file, 1);
        led_ui_post_msg("dec_pause");
    }

    if (notify) {
        req.evt.event   = AI_EVENT_PLAY_PAUSE;
        req.evt.ai_name = __this->ai_name;
        server_request(__this->ai_server, AI_REQ_EVENT, &req);
    }

    return 0;
}

static int net_music_dec_stop(int save_breakpoint)
{
    union audio_req r;
    union ai_req req;

    printf("net_music_dec_stop: %d\n", save_breakpoint);

    if (!__this->net_file) {
        return 0;
    }

    __this->dec_end_handler = NULL;

    led_ui_post_msg("dec_stop");

    net_download_buf_inactive(__this->net_file);

    if (__this->wait_download) {
        /*
         * 歌曲还未播放，删除wait
         */
        wait_completion_del(__this->wait_download);
        __this->wait_download = 0;
    } else {
        if (save_breakpoint) {
            int url_len = strlen(__this->url) + 1;
            if (url_len > __this->net_bp.url_len) {
                if (__this->net_bp.url) {
                    free(__this->net_bp.url);
                }
                __this->net_bp.url = malloc(url_len);
                if (!__this->net_bp.url) {
                    __this->net_bp.url_len = 0;
                } else {
                    __this->net_bp.url_len = url_len;
                }
            }
            if (__this->net_bp.url) {
                if (0 == __get_dec_breakpoint(&__this->net_bp.dec_bp)) {
                    __this->net_bp.ai_name = __this->ai_name;
                    strcpy(__this->net_bp.url, __this->url);
                    __this->net_bp.fptr = __this->net_bp.dec_bp.fptr;
                    log_d("save_breakpoint: fptr=%x\n", __this->net_bp.fptr);
                }
            }
        }

        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    }

    net_download_close(__this->net_file);
    __this->net_file = NULL;

    return 0;
}

static int net_music_dec_end(int save_bp)
{
    union ai_req req;

    puts("net_music_dec_end\n");

    net_music_dec_stop(save_bp);

    /*
     * 歌曲播放完成，发送此命令后ai平台会发送新的URL
     */
    req.evt.event   = AI_EVENT_MEDIA_END;
    req.evt.ai_name     = __this->ai_name;
    return server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

static int __net_music_dec_file(int breakpoint)
{
    union ai_req r;
    union audio_req req;

    __this->wait_download = 0;

    if (__this->download_ready < 0) {
        /* 网络下载失败 */
        goto __err;
    }
    if (__this->wait_switch_file) {
        puts("del_wait_switch_file\n");
        sys_timeout_del(__this->wait_switch_file);
        __this->wait_switch_file = 0;
    }
    if (__this->play_voice_prompt) {
        set_dec_end_handler(__this->file, __net_music_dec_file, breakpoint, -1);
        return 0;
    }

    req.dec.dec_type = net_download_get_media_type(__this->net_file);
    if (req.dec.dec_type == NULL) {
        goto __err;
    }
    printf("urc_file_type: %s\n", req.dec.dec_type);


    if (breakpoint == 0) {
        req.dec.bp.len = 0;
    } else {
        __this->ai_name = __this->net_bp.ai_name;
        memcpy(&req.dec.bp, &__this->net_bp.dec_bp, sizeof(req.dec.bp));
        __this->net_bp.dec_bp.len = 0;
    }

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = __this->volume;
    req.dec.output_buf      = NULL;
    req.dec.output_buf_len  = DEC_BUF_LEN;
    req.dec.file            = (FILE *)__this->net_file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = &net_audio_dec_vfs_ops;
    int err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    req.dec.cmd = AUDIO_DEC_START;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    net_download_set_pp(__this->net_file, 0);


    led_ui_post_msg("dec_start");

    return 0;

__err:
    puts("play_net_music_faild\n");

    net_download_close(__this->net_file);
    __this->net_file = NULL;

    r.evt.event     = AI_EVENT_MEDIA_END;
    r.evt.ai_name   = __this->ai_name;
    server_request(__this->ai_server, AI_REQ_EVENT, &r);

    return -EFAULT;
}

static int net_music_dec_file(void *_url, int breakpoint, void *handler, int arg)
{
    int err;
    struct net_download_parm parm;

    if (__this->dec_ops) {
        __this->dec_ops->dec_stop(0);
    }

    puts("net_download_open\n");

    memset(&parm, 0, sizeof(struct net_download_parm));
    parm.url                = (char *)_url;
    parm.prio               = 0;
    parm.cbuf_size          = 100 * 1024;
    parm.timeout_millsec    = 5000;
    parm.max_reconnect_cnt  = 5;
    parm.seek_high_range    = 0;
    parm.seek_low_range     = breakpoint ? __this->net_bp.fptr : 0;
    err = net_download_open(&__this->net_file, &parm);
    if (err) {
        log_e("net_download_open: err = %d\n", err);
        return err;
    }
    __this->url = (const char *)_url;

    set_dec_end_handler(__this->net_file, handler, arg, -1);

    /*
     * 异步等待网络下载ready，防止网络阻塞导致app卡主
     */
    __this->wait_download = wait_completion(__net_download_ready,
                                            (int (*)(void *))__net_music_dec_file, (void *)breakpoint);

    __this->dec_ops = &net_music_dec_ops;

    return 0;
}

static int net_music_dec_switch_file(int fsel_mode)
{
    int err;
    union audio_req r;
    union ai_req req;

    puts("net_music_dec_stop\n");

    net_music_dec_stop(0);

    if (fsel_mode == FSEL_NEXT_FILE) {
        req.evt.event = AI_EVENT_NEXT_SONG;
    } else if (fsel_mode == FSEL_PREV_FILE) {
        req.evt.event = AI_EVENT_PREVIOUS_SONG;
    } else {
        return 0;
    }

    req.evt.ai_name = __this->ai_name;
    err = server_request(__this->ai_server, AI_REQ_EVENT, &req);
    if (err) {
        /*
         * 上下曲失败，播放本地文件
         */
        local_music_dec_breakpoint(0);
    } else {
        /*
         * 设置超时，网络异常导致切歌失败时播放本地文件
         */
        if (__this->wait_switch_file == 0) {
            __this->wait_switch_file = sys_timeout_add(NULL,
                                       (void (*)(void *))app_music_switch_local_device, 10000);
        }
    }

    return 0;
}

static int net_music_dec_switch_dir(int fsel_mode)
{
    return 0;
}

static int net_music_dec_breakpoint(int priv)
{
    puts("net_music_dec_breakpoint\n");

    if (__this->net_bp.dec_bp.len == 0) {
        return -ENOENT;
    }
    net_music_dec_file(__this->net_bp.url, 1, net_music_dec_end, 0);

    return 0;
}

static int net_music_dec_progress(int sec)
{
    union ai_req req;

    req.evt.event       = AI_EVENT_PLAY_TIME;
    req.evt.progress    = sec;
    req.evt.ai_name     = __this->ai_name;
    server_request(__this->ai_server, AI_REQ_EVENT, &req);

    return 0;
}

static int net_music_dec_volume(int step)
{
    int err;
    union ai_req req;

    err = __set_dec_volume(step);
    if (err) {
        return err;
    }
    req.evt.event       = AI_EVENT_VOLUME_CHANGE;
    req.evt.volume      = __this->volume;
    req.evt.ai_name     = __this->ai_name;
    return server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

static const struct music_dec_ops net_music_dec_ops = {
    .switch_dir     = net_music_dec_switch_dir,
    .switch_file    = net_music_dec_switch_file,
    .dec_file       = net_music_dec_file,
    .dec_breakpoint = net_music_dec_breakpoint,
    .dec_play_pause = net_music_dec_play_pause,
    .dec_volume     = net_music_dec_volume,
    .dec_progress   = net_music_dec_progress,
    .dec_stop       = net_music_dec_stop,
};

static void ai_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AI_SERVER_EVENT_CONNECTED:
        if (!strcmp((const char *)argv[1], "wechat")) {
            if (!__this->wechat_connected) {
                __this->wechat_connected = 1;
            }
        } else {
            if (!__this->ai_connected) {
                __this->ai_connected = 1;
                app_music_play_voice_prompt("002.mp3", __this->dec_ops->dec_breakpoint);
            }
        }
        break;
    case AI_SERVER_EVENT_DISCONNECTED:
        if (!strcmp((const char *)argv[1], "wechat")) {

        } else {
            if (__this->ai_connected) {
                __this->ai_connected = 0;
                app_music_play_voice_prompt("014.mp3", __this->dec_ops->dec_breakpoint);
            }
        }
        break;
    case AI_SERVER_EVENT_URL:
        __this->ai_name = (const char *)argv[2];
        if (__this->listening == LISTEN_STATE_START) {
            if (!strcmp(__this->ai_name, "wechat")) {
                return;
            }
            __this->listening = LISTEN_STATE_RETURN_URL;
        }


        __this->dec_ops->dec_stop(1);
        net_music_dec_file((void *)argv[1], 0, net_music_dec_end, 0);
        break;
    case AI_SERVER_EVENT_CONTINUE:
        __this->dec_ops->dec_play_pause(0);
        break;
    case AI_SERVER_EVENT_PAUSE:
        __this->dec_ops->dec_play_pause(0);
        break;
    case AI_SERVER_EVENT_UPGRADE:
        sys_power_auto_shutdown_pause();
        app_music_play_voice_prompt("024.mp3", NULL);
        break;
    default:
        break;
    }
}

static void app_music_ai_listen_start(u8 wechat)
{
    int err;
    union ai_req req;

    if (__this->listening != LISTEN_STATE_STOP) {
        return;
    }

    if (!__this->ai_connected) {
        if (wechat) {
            app_music_play_voice_prompt("016.mp3", __this->dec_ops->dec_breakpoint);
        } else {
            app_music_play_voice_prompt("015.mp3", __this->dec_ops->dec_breakpoint);
        }
        return;
    }

    if (__this->dec_ops) {
        __this->dec_ops->dec_stop(1);
    }
    req.evt.event   = AI_EVENT_MEDIA_STOP;
    req.evt.ai_name = __this->ai_name;
    server_request(__this->ai_server, AI_REQ_EVENT, &req);

    app_music_play_voice_prompt("rec.mp3", NULL);

    if (wechat) {
        os_time_dly(50);
        req.evt.event   = AI_EVENT_VOICE_MODE;
        req.evt.ai_name = "duer";
        server_request(__this->ai_server, AI_REQ_EVENT, &req);
    }

    req.lis.cmd = AI_LISTEN_START;
    err = server_request(__this->ai_server, AI_REQ_LISTEN, &req);
    if (err == 0) {
        __this->listening = LISTEN_STATE_START;
    }
}

static void app_music_ai_listen_stop()
{
    union ai_req req;

    if (__this->listening != LISTEN_STATE_START) {
        __this->listening = LISTEN_STATE_STOP;
        return;
    }

    __this->listening = LISTEN_STATE_STOP;

    if (__this->ai_server) {
        req.lis.cmd = AI_LISTEN_STOP;
        server_request(__this->ai_server, AI_REQ_LISTEN, &req);
    }
    app_music_play_voice_prompt("send.mp3", NULL);
    os_time_dly(5);
}

static void app_music_event_net_connected()
{
    union ai_req req;

    /*
     * 网络连接成功,开始连接ai服务器
     */
    if (__this->ai_server) {
        return;
    }

    __this->ai_server = server_open("ai_server", NULL);
    if (!__this->ai_server) {
        return;
    }

    server_register_event_handler(__this->ai_server, NULL, ai_server_event_handler);

    server_request(__this->ai_server, AI_REQ_CONNECT, &req);
}


/************************微信相关API****************************/
extern int net_dhcp_ready();
extern char jieliapp_net_ready(void);

static int wechat_start(void)
{
    struct wechat_req req;
    req.cmd = WECHAT_STATE_START;
    server_request(__this->wechat_server, WECHAT_REQ, &req);

    return 0;
}

static int wechat_open(void)
{
    struct wechat_req req;
    req.cmd = WECHAT_STATE_OPEN;
    server_request(__this->wechat_server, WECHAT_REQ, &req);

    return 0;
}

static int app_wechat_enc_start()
{
    int err;
    struct wechat_req req;

    if (!__this->wechat_connected) {
        app_music_play_voice_prompt("016.mp3", __this->dec_ops->dec_breakpoint);
        return 0;
    }
    if (__this->wechat_state & 0x1) {
        return -1;
    }

    if (__this->dec_ops) {
        __this->dec_ops->dec_stop(1);
    }

    app_music_play_voice_prompt("rec.mp3", NULL);

    os_time_dly(50); /*防止按键声音*/

    req.cmd = WECHAT_STATE_ENC_AMR_START;
    err = server_request(__this->wechat_server, WECHAT_REQ, &req);
    if (err == 0) {
        __this->wechat_state |= 0x1;
    }

    return err;
}

static int app_wechat_enc_stop()
{
    struct wechat_req req;
    int err;

    if (!(__this->wechat_state & 0x1)) {
        return 0;
    }

    req.cmd = WECHAT_STATE_ENC_AMR_STOP;
    err = server_request(__this->wechat_server, WECHAT_REQ, &req);
    printf("\n\napp_wechat_amr_enc_stop err = %d\n\n", err);

    __this->wechat_state &= ~0x1;

    app_music_play_voice_prompt("send.mp3", NULL);

    return 0;
}

int wechat_music_dec_volume(int step)
{
    int err;
    union audio_req req;

    int volume = step;
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    if (volume == __this->volume) {
        return -EINVAL;
    }
    __this->volume = volume;

    log_d("set_dec_volume: %d\n", volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = volume;
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err == 0) {
#ifdef CONFIG_STORE_VOLUME
        db_update("vol", __this->volume);
        db_flush();
#endif

    }
    return err;

}

int get_app_music_volume(void)
{
    return __this->volume;
}

int get_app_music_playtime(void)
{
    return __this->play_time;
}

void duer_wechat_speak_play(const char *url)
{
    strcpy(__this->wechat_speech_url, url);
    __this->wechat_speech_read_flag = 1;
    app_music_play_voice_prompt("009.mp3", __this->dec_ops->dec_breakpoint);
}

void duer_wechat_err_notify(void)
{
    os_time_dly(50);
    app_music_play_voice_prompt("019.mp3", __this->dec_ops->dec_breakpoint);
}

static void wechat_server_event_handler(void *priv, int argc, int *argv)
{
    char buffer[7];
    switch (argv[0]) {
    case WECHAT_SERVER_SPEECH_URL_EVENT:
        printf("WECHAT_SERVER_SPEECH_URL_EVENT\n");
        strcpy(__this->wechat_speech_url, (char *)argv[1]);
        __this->wechat_speech_read_flag = 1;
        app_music_play_voice_prompt("009.mp3", __this->dec_ops->dec_breakpoint);
        break;
    case WECHAT_SERVER_AMR_ERR_EVENT:
        printf("WECHAT_SERVER_AMR_ERR_EVENT\n");
        app_music_play_voice_prompt("019.mp3", __this->dec_ops->dec_breakpoint);
    case WECHAT_SERVER_AMR_END_EVENT:
        printf("WECHAT_SERVER_AMR_END_EVENT\n");
        app_wechat_enc_stop(); //超时可增加提示音
        break;
    }
}


/***************************************************************/
extern int dev_profile_init(void);
extern void dev_profile_uninit(void);
extern void dev_profile_update(void);
extern void wifi_enter_smp_cfg_mode(void);
extern void wifi_return_sta_mode(void);
extern int get_wifi_is_smp_mode(void);

static void app_music_net_config(void)
{
    sys_power_auto_shutdown_pause();

    if (!__this->wifi_config_state) {
        if (__this->ai_server) {
            server_close(__this->ai_server);
            __this->ai_server = NULL;
            __this->ai_connected = 0;
            __this->wechat_connected = 0;
        }
        dev_profile_uninit();
        app_music_play_voice_prompt("013.mp3", NULL);
        wifi_enter_smp_cfg_mode();
    } else {
        if (get_wifi_is_smp_mode()) {
            wifi_return_sta_mode();
        }
        app_music_play_voice_prompt("017.mp3", NULL);
    }
    __this->wifi_config_state = !__this->wifi_config_state;
}

#endif




static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        puts("app_music: AUDIO_SERVER_EVENT_ERR\n");
    case AUDIO_SERVER_EVENT_END:
        puts("app_music: AUDIO_SERVER_EVENT_END\n");
        led_ui_post_msg("dec_end");
        sys_key_event_enable();
        __this->play_voice_prompt = 0;
        do_dec_end_handler((void *)argv[1]);
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        log_d("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
        sys_power_auto_shutdown_clear();
        /* __this->dec_ops->dec_progress(argv[1]); */
        break;
    }
}

static int __play_voice_prompt(const char *fname, void *dec_end_handler, int save_bp)
{
    int err;
    char path[64];

    log_d("play_voice_prompt: %s\n", fname);

    sprintf(path, "%s%s", CONFIG_VOICE_PROMPT_FILE_PATH, fname);

    FILE *file = fopen(path, "r");
    if (!file) {
        return -ENOENT;
    }
    sys_key_event_disable();

    __this->dec_ops->dec_stop(save_bp);

    __this->play_voice_prompt = 1;

    err = local_music_dec_file(file, 0, dec_end_handler, 0);
    if (err) {
        sys_key_event_enable();
    }

    return err;
}

static int _play_voice_prompt(const char *fname, void *dec_end_handler)
{
    return __play_voice_prompt(fname, dec_end_handler, 0);
}

/*
 * 播放语音提示
 */
static int app_music_play_voice_prompt(const char *fname, void *dec_end_handler)
{
    if (dec_end_handler == (void *)app_music_shutdown) {
        return _play_voice_prompt(fname, dec_end_handler);
    }

    if (__this->play_voice_prompt) {
        if (__this->dec_end_handler != (void *)app_music_shutdown) {
            set_dec_end_handler(__this->dec_end_file, _play_voice_prompt,
                                (int)fname, (int)dec_end_handler);
        }
        return 0;
    }

    return __play_voice_prompt(fname, dec_end_handler, 1);
}


static int app_music_main()
{
    int err;
    puts("action_music_play_main\n");

#ifdef CONFIG_STORE_VOLUME
    __this->volume = db_select("vol");
#endif
    if (__this->volume <= 0 || __this->volume > 100) {
        __this->volume = 60;
    }

    led_ui_post_msg("init");
    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        puts("play_music open audio_server fail!\n");
        return -EPERM;
    }
    server_register_event_handler(__this->dec_server, NULL, dec_server_event_handler);

#ifdef CONFIG_NET_ENABLE
#ifdef CONFIG_PROFILE_UPDATE
    dev_profile_update();
#endif

#ifndef CONFIG_DUER_WECHAT_ENABLE
    __this->wechat_server = server_open("wechat_server", NULL);
    if (!__this->wechat_server) {
        puts("play_music open wechat_server fail!\n");
        return -EPERM;
    }

    server_register_event_handler(__this->wechat_server, NULL, wechat_server_event_handler);

    wait_completion(net_dhcp_ready, (int (*)(void *))wechat_open, NULL);
    wait_completion(net_dhcp_ready, (int (*)(void *))wechat_start, NULL);
#endif
#endif

    /*
     * 播放开机提示音
     */
    FILE *file = fopen(CONFIG_VOICE_PROMPT_FILE_PATH"001.mp3", "r");
    if (file) {
        sys_key_event_disable();
        err = local_music_dec_file(file, 0, app_music_switch_local_device, 0);
        if (err) {
            sys_key_event_enable();
        }
    } else {
        err = app_music_switch_local_device(0);
    }

    return err;
}

/*
 *按键响应函数
 */
static int app_music_key_click(struct key_event *key)
{
    switch (key->value) {
    case KEY_OK:
        if (__this->dec_ops && !__this->play_voice_prompt) {
            __this->dec_ops->dec_play_pause(1);
        }
        break;
    case KEY_UP:
        puts("music key up\n");
        if (__this->dec_ops) {
            puts("prev_file\n");
            __this->dec_ops->switch_file(FSEL_PREV_FILE);
        }
        break;
    case KEY_DOWN:
        puts("music key down\n");
        if (__this->dec_ops) {
            __this->dec_ops->switch_file(FSEL_NEXT_FILE);
        }
        break;
#ifdef CONFIG_NET_ENABLE
    case KEY_ENC:
        if (!__this->ai_connected) {
            app_music_play_voice_prompt("015.mp3", __this->dec_ops->dec_breakpoint);
        } else {
            app_music_play_voice_prompt("003.mp3", NULL);
        }
        break;
    case KEY_F1:
#ifndef CONFIG_DUER_WECHAT_ENABLE
        if (!__this->wechat_connected) {
#else
        if (!__this->ai_connected) {
#endif
            app_music_play_voice_prompt("016.mp3", __this->dec_ops->dec_breakpoint);
        } else {
            if (__this->wechat_speech_read_flag) {
                /*微信语音播完不通知AI_SERVER */
                __this->dec_ops->dec_stop(1);
                net_music_dec_file(__this->wechat_speech_url, 0,
                                   __this->dec_ops->dec_breakpoint, 0);
                memset(__this->wechat_speech_url, 0, sizeof(__this->wechat_speech_url));
                __this->wechat_speech_read_flag = 0;
            } else {
                app_music_play_voice_prompt("023.mp3", NULL);
            }
        }
#endif
        break;
    default:
        break;
    }
    return false;
}

static int app_music_key_hold(struct key_event *key)
{
    switch (key->value) {
    case KEY_UP:
        if (__this->dec_ops) {
            if (0 == __this->dec_ops->dec_volume(-3)) {
#ifdef CONFIG_STORE_VOLUME
                db_update("vol", __this->volume);
                db_flush();
#endif
            }
        }
        break;
    case KEY_DOWN:
        if (__this->dec_ops) {
            if (0 == __this->dec_ops->dec_volume(3)) {
#ifdef CONFIG_STORE_VOLUME
                db_update("vol", __this->volume);
                db_flush();
#endif
            }
        }
        break;
#ifdef CONFIG_NET_ENABLE
    case KEY_ENC:
        app_music_ai_listen_start(0);
        break;
    case KEY_F1:  //wechat
#ifdef CONFIG_DUER_WECHAT_ENABLE
        app_music_ai_listen_start(1);
#else
        app_wechat_enc_start();
#endif
        break;
#endif
    }
    return false;
}

static int app_music_key_event_handler(struct key_event *key)
{
    int ret = false;
    union audio_req r;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        ret = app_music_key_click(key);
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_OK) {
            puts("switch_next_dir\n");
            local_music_dec_switch_dir(FSEL_NEXT_FILE);
            /*切换本地通知AI_SERVER  AI_EVENT_MEDIA_STOP 事件*/
#ifdef CONFIG_NET_ENABLE
            union ai_req req = {0};
            req.evt.event   = AI_EVENT_MEDIA_STOP;
            req.evt.ai_name     = __this->ai_name;
            server_request(__this->ai_server, AI_REQ_EVENT, &req);

#endif

        } else if (key->value == KEY_MODE) {
#ifdef CONFIG_NET_ENABLE
            puts("switch_net_config\n");
            app_music_net_config();
#endif
        }
        break;
    case KEY_EVENT_HOLD:
        ret = app_music_key_hold(key);
        break;
    case KEY_EVENT_UP:
#ifdef CONFIG_NET_ENABLE
        if (key->value == KEY_ENC) {
            app_music_ai_listen_stop();
        } else if (key->value == KEY_F1) {  //wechat
#ifdef CONFIG_DUER_WECHAT_ENABLE
            app_music_ai_listen_stop();
#else
            app_wechat_enc_stop();
#endif
        }
#endif
        break;
    default:
        break;
    }

    return ret;
}

static void app_music_low_power(void *priv)
{
    sys_timer_modify(__this->low_power, 60000);
    app_music_play_voice_prompt("021.mp3", __this->dec_ops->dec_breakpoint);
}

/*
 *设备响应函数
 */
static int app_music_device_event_handler(struct sys_event *event)
{
    /*
     * SD卡插拔处理
     */
    if (!ASCII_StrCmp(event->arg, SDX_DEV, 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            wait_completion(storage_device_ready,
                            (int (*)(void *))app_music_switch_local_device,
                            CONFIG_MUSIC_PATH_SD);
            break;
        case DEVICE_EVENT_OUT:
            if (!strcmp(__this->local_path, CONFIG_MUSIC_PATH_SD)) {
                if (__this->dec_ops == &local_music_dec_ops) {
                    app_music_switch_local_device(CONFIG_MUSIC_PATH_FLASH);
                } else {
                    __this->local_bp.len = 0;
                    __this->local_path = NULL;
                }
            }
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "sys_power", 9)) {
        if (event->u.dev.event == DEVICE_EVENT_POWER_PERCENT) {
            int battery_val = event->u.dev.value;
            if (battery_val <= 30 && !sys_power_is_charging()) {
                if (__this->low_power == 0) {
                    __this->low_power = sys_timer_add(NULL,
                                                      app_music_low_power, 5000);
                }
            } else {
                if (__this->low_power) {
                    sys_timer_del(__this->low_power);
                    __this->low_power = 0;
                }
            }
        } else if (event->u.dev.event == DEVICE_EVENT_POWER_SHUTDOWN) {
            if (!strcmp((char *)event->u.dev.value, "auto")) {
                app_music_play_voice_prompt("018.mp3", app_music_shutdown);
            } else if (!strcmp((char *)event->u.dev.value, "low_power")) {
                log_d("low_power_shutdown\n");
                app_music_play_voice_prompt("022.mp3", app_music_shutdown);
            }
        }
    }
    return false;
}

static int app_music_net_event_handler(struct sys_event *event)
{
#ifdef CONFIG_NET_ENABLE
    if (!ASCII_StrCmp(event->arg, "net", 4)) {
        switch (event->u.net.event) {
        case NET_EVENT_CONNECTED:
            puts("NET_EVENT_CONNECTED\n");
            if (!get_wifi_is_smp_mode()) {
                __this->wifi_config_state =  0;
                dev_profile_init();
                app_music_event_net_connected();
            }
            break;
        case NET_EVENT_DISCONNECTED:
            puts("NET_EVENT_DISCONNECTED\n");
            return false;
        case NET_EVENT_SMP_CFG_TIMEOUT:
            if (__this->wifi_config_state) {
                app_music_play_voice_prompt("020.mp3", local_music_dec_breakpoint);
                __this->wifi_config_state =  0;
            }
        default:
            break;
        }
        sys_power_auto_shutdown_resume();
    }
#endif

    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return app_music_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return app_music_device_event_handler(event);
    case SYS_NET_EVENT:
        return app_music_net_event_handler(event);
    default:
        return false;
    }
}

static int state_machine(struct application *app, enum app_state state,
                         struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        memset(__this, 0, sizeof(struct app_music_hdl));
#ifdef CONFIG_NET_ENABLE
        __this->net_bp.url = malloc(512);
        __this->net_bp.url_len = 512;
#endif

        __this->led_ui = server_open("led_ui_server", NULL);
        if (!__this->led_ui) {
            log_e("led_ui open server err!\n");
            return -EINVAL;
        }
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_MUSIC_PLAY_MAIN:
            app_music_main();
            break;
        case ACTION_MUSIC_PLAY_VOICE_PROMPT:
            app_music_play_voice_prompt(it->data, __this->dec_ops->dec_breakpoint);
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        if (__this->led_ui) {
            server_close(__this->led_ui);
            __this->led_ui = NULL;
        }
        break;
    }

    return 0;
}

static const struct application_operation app_music_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

REGISTER_APPLICATION(app_app_music) = {
    .name 	= "app_music",
    .action	= ACTION_MUSIC_PLAY_MAIN,
    .ops 	= &app_music_ops,
    .state  = APP_STA_DESTROY,
};


