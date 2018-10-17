#include "http/http_cli.h"
#include "wechat/dev_net_mqtt.h"
#include "wechat/dev_net_oauth.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "os/os_api.h"
#include "os/os_compat.h"
#include "wechat/wechat_server.h"
#include "system/database.h"
#include "device/device.h"
#include "wifi_dev.h"
#include "fs/fs.h"
#include "storage_device.h"
#include "sock_api/sock_api.h"
#include "app_config.h"


#ifdef CONFIG_WECHAT_SDK_ENABLE
#define RECV_DATA_SIZE	1024

struct wechat_html_file_info {
    char filename[128];
    char filetime[16];
    char fileurl[1024];
    char meta_uuid[128];
    u32 album_id;
    u32 meta_sn;
    u32 state;
};

enum {
    WECHAT_MUSIC_PLAYING = 0x1,
    WECHAT_MUSIC_PAUSE,
    WECHAT_MUSIC_STOP,
    WECHAT_MUSIC_DOWNLOADING,
    WECHAT_MUSIC_DOWNLOAD_FLINSH,

};

struct __wechat_html_status {
    struct wechat_html_file_info cur;
    struct wechat_html_file_info playlist;
};

static struct __wechat_html_status wechat_html_status;

#define __this (&wechat_html_status)


extern void JL_wechat_media_audio_play(const char *url);
extern void JL_wechat_media_audio_continue(const char *url);
extern void JL_wechat_media_audio_pause(const char *url);
extern int wechat_music_dec_volume(int step);
extern char *itoa(int num, char *str, int radix);
extern int atoi(const char *__nptr);
extern int wechat_server_event_url(const char *url, int event);


/*微信点播*/
static void app_mqtt_cb_user_play(const char *buffer)
{
    json_object *new_obj1 = NULL;
    json_object *url = NULL;
    json_object *name = NULL;
    json_object *time = NULL;
    json_object *album_id = NULL;
    json_object *meta_sn = NULL;

    new_obj1 = json_tokener_parse(buffer);

    if (!json_object_object_get_ex(new_obj1, "url", &url)) {
        goto __result_exit;
    }

    const char *url_string = json_object_get_string(url);

    if (!json_object_object_get_ex(new_obj1, "name", &name)) {
        goto __result_exit;
    }

    const char *name_string = json_object_get_string(name);

    if (strlen(name_string) > sizeof(__this->cur.filename) - 1) {
        memcpy(__this->cur.filename, name_string, sizeof(__this->cur.filename) - 1);
        __this->cur.filename[sizeof(__this->cur.filename) - 1] = 0;
    } else {
        strcpy(__this->cur.filename, name_string);
    }

    if (strlen(url_string) > sizeof(__this->cur.fileurl) - 1) {
        memcpy(__this->cur.fileurl, url_string, sizeof(__this->cur.fileurl) - 1);
        __this->cur.fileurl[sizeof(__this->cur.fileurl) - 1] = 0;
    } else {
        strcpy(__this->cur.fileurl, url_string);
    }


    if (!json_object_object_get_ex(new_obj1, "time", &time)) {
        goto __result_exit;
    }

    const char *time_string = json_object_get_string(time);

    strcpy(__this->cur.filetime, time_string);

    if (!json_object_object_get_ex(new_obj1, "album_id", &album_id)) {
        goto __result_exit;
    }

    __this->cur.album_id = json_object_get_int(album_id);

    if (!json_object_object_get_ex(new_obj1, "meta_sn", &meta_sn)) {
        goto __result_exit;
    }

    __this->cur.meta_sn = json_object_get_int(meta_sn);

    printf("album_id=%d  meta_sn:%d\n", __this->cur.album_id, __this->cur.meta_sn);

    dev_mqtt_push_status("play", __this->cur.filename);
    __this->cur.state = WECHAT_MUSIC_PLAYING;

    dev_mqtt_push_status("play_status", "play");
    //to do something
    JL_wechat_media_audio_play(url_string);

__result_exit:
    json_object_put(new_obj1);
}

static void app_mqtt_cb_user_next(const char *buffer)
{
    u8 *data;
    json_object *new_obj1 = NULL;
    json_object *data1 = NULL;
    json_object *url = NULL;
    json_object *album_id = NULL;
    json_object *meta_sn = NULL;
    json_object *name = NULL;
    json_object *time = NULL;
    if (!__this->cur.state) {
        return;
    }


    data = (u8 *)calloc(1, RECV_DATA_SIZE);
    if (!data) {
        return;
    }

    if (app_get_music_url("next", __this->cur.album_id, __this->cur.meta_sn, data, RECV_DATA_SIZE)) {
        free(data);
        return;
    }

    new_obj1 = json_tokener_parse((const char *)data);
    if (!json_object_object_get_ex(new_obj1, "data", &data1)) {
        goto __result_exit;
    }

    if (!json_object_object_get_ex(data1, "url", &url)) {
        goto __result_exit;
    }

    const char *url_string = json_object_get_string(url);

    if (!json_object_object_get_ex(data1, "name", &name)) {
        goto __result_exit;
    }

    const char *name_string = json_object_get_string(name);

    if (strlen(name_string) > sizeof(__this->cur.filename) - 1) {
        memcpy(__this->cur.filename, name_string, sizeof(__this->cur.filename) - 1);
        __this->cur.filename[sizeof(__this->cur.filename) - 1] = 0;
    } else {
        strcpy(__this->cur.filename, name_string);
    }

    if (strlen(url_string) > sizeof(__this->cur.fileurl) - 1) {
        memcpy(__this->cur.fileurl, url_string, sizeof(__this->cur.fileurl) - 1);
        __this->cur.fileurl[sizeof(__this->cur.fileurl) - 1] = 0;
    } else {
        strcpy(__this->cur.fileurl, url_string);
    }



    if (!json_object_object_get_ex(data1, "time", &time)) {
        goto __result_exit;
    }

    const char *time_string = json_object_get_string(time);

    strcpy(__this->cur.filetime, time_string);

    if (!json_object_object_get_ex(data1, "album_id", &album_id)) {
        goto __result_exit;
    }

    __this->cur.album_id = json_object_get_int(album_id);

    if (!json_object_object_get_ex(data1, "meta_sn", &meta_sn)) {
        goto __result_exit;
    }

    __this->cur.meta_sn = json_object_get_int(meta_sn);

    printf("album_id=%d  meta_sn:%d\n", __this->cur.album_id, __this->cur.meta_sn);

    JL_wechat_media_audio_play(url_string);

    dev_mqtt_push_status("play", __this->cur.filename);
    __this->cur.state = WECHAT_MUSIC_PLAYING;
    dev_mqtt_push_status("play_status", "play");

__result_exit:

    json_object_put(new_obj1);
    free(data);

}

static void app_mqtt_cb_user_pre(const char *buffer)
{
    u8 *data;
    json_object *new_obj1 = NULL;
    json_object *data1 = NULL;
    json_object *url = NULL;
    json_object *album_id = NULL;
    json_object *meta_sn = NULL;
    json_object *name = NULL;
    json_object *time = NULL;
    if (!__this->cur.state) {
        return;
    }

    data = (u8 *)calloc(1, RECV_DATA_SIZE);
    if (!data) {
        return;
    }

    if (app_get_music_url("pre", __this->cur.album_id, __this->cur.meta_sn, data, RECV_DATA_SIZE)) {
        free(data);
        return;
    }

    new_obj1 = json_tokener_parse((const char *)data);
    if (!json_object_object_get_ex(new_obj1, "data", &data1)) {
        goto __result_exit;
    }

    if (!json_object_object_get_ex(data1, "url", &url)) {
        goto __result_exit;
    }

    const char *url_string = json_object_get_string(url);

    if (!json_object_object_get_ex(data1, "name", &name)) {
        goto __result_exit;
    }

    const char *name_string = json_object_get_string(name);

    if (strlen(name_string) > sizeof(__this->cur.filename) - 1) {
        memcpy(__this->cur.filename, name_string, sizeof(__this->cur.filename) - 1);
        __this->cur.filename[sizeof(__this->cur.filename) - 1] = 0;
    } else {
        strcpy(__this->cur.filename, name_string);
    }


    if (strlen(url_string) > sizeof(__this->cur.fileurl) - 1) {
        memcpy(__this->cur.fileurl, url_string, sizeof(__this->cur.fileurl) - 1);
        __this->cur.fileurl[sizeof(__this->cur.fileurl) - 1] = 0;
    } else {
        strcpy(__this->cur.fileurl, url_string);
    }




    if (!json_object_object_get_ex(data1, "time", &time)) {
        goto __result_exit;
    }

    const char *time_string = json_object_get_string(time);

    strcpy(__this->cur.filetime, time_string);

    if (!json_object_object_get_ex(data1, "album_id", &album_id)) {
        goto __result_exit;
    }

    __this->cur.album_id = json_object_get_int(album_id);

    if (!json_object_object_get_ex(data1, "meta_sn", &meta_sn)) {
        goto __result_exit;
    }

    __this->cur.meta_sn = json_object_get_int(meta_sn);

    printf("album_id=%d  meta_sn:%d\n", __this->cur.album_id, __this->cur.meta_sn);

    JL_wechat_media_audio_play(url_string);
    dev_mqtt_push_status("play", __this->cur.filename);
    __this->cur.state = WECHAT_MUSIC_PLAYING;

    dev_mqtt_push_status("play_status", "play");

__result_exit:
    json_object_put(new_obj1);
    free(data);

}


static void app_mqtt_cb_user_pause(const char *buffer)
{
    char buf[10];
    if (!__this->cur.state) {
        return;
    }

    if (__this->cur.state == WECHAT_MUSIC_PLAYING) {
        strcpy(buf, "stop");
        __this->cur.state = WECHAT_MUSIC_PAUSE;
    } else if (__this->cur.state == WECHAT_MUSIC_PAUSE
               || __this->cur.state == WECHAT_MUSIC_STOP) {

        strcpy(buf, "play");
        __this->cur.state = WECHAT_MUSIC_PLAYING;
    }
    dev_mqtt_push_status("play_status", buf);
}


static void app_mqtt_cb_user_stop(const char *buffer)
{
    char buf[10];
    if (!__this->cur.state) {
        return;
    }

    __this->cur.state = WECHAT_MUSIC_STOP;
    strcpy(buf, "stop");
    dev_mqtt_push_status("play_status", buf);
}




static void app_mqtt_cb_user_volume_change(const char *buffer)
{
#if 0
    static u32 old_vol = 0;
    char buf[16];
    u32 vol = db_select("vol");
    if (vol - old_vol > 20  ||  old_vol - vol > 20) {
        itoa(vol, buf, 10);
        dev_mqtt_push_status("volume", buf);
    }
    old_vol = vol;
#endif
}

static void app_mqtt_cb_user_update(const char *buffer)
{
    /* if (!__this->cur.state) { */
    /* return; */
    /* } */
    printf("\n\n\nupdate update update\n\n\n");
    ota_update("wifi_story", "device", OTA_MAJOR, OTA_MINOR, OTA_PATCH);
}




/*微信语音*/
static void app_mqtt_cb_sys_speech(const char *buffer)
{
    //to do something
    wechat_server_event_url(buffer, WECHAT_SERVER_SPEECH_URL_EVENT);
}

static void app_mqtt_cb_user_volume(const char *buffer)
{
    printf("buffer->%s\n", buffer);
    wechat_music_dec_volume(atoi(buffer));
}
/*********************playlist实现***********************************/
extern httpin_error httpcli_init(httpcli_ctx *ctx);
extern s32_t httpcli_read(httpcli_ctx *ctx, char *recvbuf, u32_t len);
extern void httpcli_close(httpcli_ctx *ctx);


int httpcli_b(void *ctx, void *buf, unsigned int size, void *priv, httpin_status status)
{
    return 0;

}


static int __http_playlist_download(const char *url, const char *name)
{
    int error = 0;
    httpcli_ctx ctx;
    int ret = 0;
    void *fd = NULL;
    char *buffer = NULL;
    char namebuf[256];

    const char story_path[8] = { 0x45, 0x65, 0x8B, 0x4E, 0x2f, 0x0}; //故事
    printf("url->%s\n", url);
    memset(&ctx, 0x0, sizeof(ctx));
    memset(namebuf, 0, 256);

    ret = fname_to_path(namebuf, CONFIG_ROOT_PATH, story_path, 6);
    put_buf(namebuf, ret);
    memcpy(namebuf + ret, name, strlen(name));
    put_buf(namebuf, ret + strlen(name));
    memcpy(namebuf + ret + strlen(name), url + strlen(url) - 4, 4);
    ctx.url = url;
    ctx.connection = "close";
    ctx.timeout_millsec = 2000;
    ctx.cb = httpcli_b;
    error = httpcli_init(&ctx);
    if (error == HERROR_OK) {
        fd = fopen(namebuf, "w+");
        if (!fd) {
            printf("open  file fail\n");
        } else {
            buffer = malloc(8 * 1024);
            if (!buffer) {
                return -1;
            }
            while (ctx.content_length) {
                ret = httpcli_read(&ctx, buffer, 8 * 1024);
                if (ret < 0) {
                    return -1;
                }
                fwrite(fd, buffer, ret);
                ctx.content_length -= ret ;
                /* printf("ctx.content_length=%d\n", ctx.content_length); */
            }

        }

    }

    fclose(fd);
    free(buffer);
    /* free(namebuf); */
    /* free(namebuf1); */
    /* free(namebuf2); */

    httpcli_close(&ctx);

    return error;


}




static void playlist_download_task(void *arg)
{
    struct wechat_html_file_info *playlist = (struct wechat_html_file_info *)arg;

    int ret = 0;

    printf("url=>%s\n", playlist->fileurl);
    printf("filename=>%s\n", playlist->filename);
    printf("meta_uuid=>%s\n", playlist->meta_uuid);
    //下载中状态
    char *buffer = malloc(1024);

    sprintf(buffer, "{\\\"type\\\":\\\"download\\\",\\\"info\\\":\\\"正在下载：%s\\\"}", playlist->filename);
    dev_mqtt_push_playlist("download", buffer);
    ret = __http_playlist_download(playlist->fileurl, playlist->meta_uuid);
    if (ret) {
        printf("http playlist download fail\n");
        //下载失败,没实现
        sprintf(buffer, "{\\\"type\\\":\\\"error\\\",\\\"info\\\":\\\"下载失败：%s\\\"}", playlist->filename);
        dev_mqtt_push_playlist("download", buffer);
    } else  {
        //下载完
        sprintf(buffer, "{\\\"type\\\":\\\"finish\\\",\\\"info\\\":\\\"下载完成：%s\\\"}", playlist->filename);
        dev_mqtt_push_playlist("download", buffer);
    }


    __this->playlist.state = WECHAT_MUSIC_DOWNLOAD_FLINSH;
    free(buffer);

}

static void app_mqtt_cb_user_playlist_download(const char *buffer)
{
    printf("buffer->%s\n", buffer);
    int i;
    int ret;
    json_object *new_obj1 = NULL;
    json_object *new_obj2 = NULL;
    json_object *url = NULL;
    json_object *name = NULL;
    json_object *meta_uuid = NULL;

    char *buffer1 = malloc(1024);

    new_obj1 = json_tokener_parse((const char *)buffer);

    for (i = 0;; i++) {
        new_obj2 =   array_list_get_idx(json_object_get_array(new_obj1), i);
        if (new_obj2 == NULL) {
            break;
        }

        if (!json_object_object_get_ex(new_obj2, "url", &url)) {
            goto __result_exit;
        }
        const char *url_string = json_object_get_string(url);

        /* strcpy(__this->fileurl,url_string); */
        /* printf("url=>%s\n", time_string); */
        if (strlen(url_string) > sizeof(__this->playlist.fileurl) - 1) {
            memcpy(__this->playlist.fileurl, url_string, sizeof(__this->playlist.fileurl) - 1);
            __this->playlist.fileurl[sizeof(__this->playlist.fileurl) - 1] = 0;
        } else {
            strcpy(__this->playlist.fileurl, url_string);
        }


        if (!json_object_object_get_ex(new_obj2, "meta_uuid", &meta_uuid)) {
            goto __result_exit;
        }
        const char *meta_uuid_string = json_object_get_string(meta_uuid);



        //只下载一首
        if (!strcmp(meta_uuid_string, __this->playlist.meta_uuid)
            || __this->playlist.state == WECHAT_MUSIC_DOWNLOADING) {
            sprintf(buffer1, "{\\\"type\\\":\\\"download\\\",\\\"info\\\":\\\"正在下载：%s\\\"}", __this->playlist.filename);
            dev_mqtt_push_playlist("download", buffer1);
            printf("warnning it is downloading.....\n");
            goto __result_exit;
        }


        if (!json_object_object_get_ex(new_obj2, "name", &name)) {
            goto __result_exit;
        }
        const char *name_string = json_object_get_string(name);

        if (strlen(name_string) > sizeof(__this->playlist.filename) - 1) {
            memcpy(__this->playlist.filename, name_string, sizeof(__this->playlist.filename) - 1);
            __this->playlist.filename[sizeof(__this->playlist.filename) - 1] = 0;
        } else {
            strcpy(__this->playlist.filename, name_string);
        }

        //检测SD状态
        if (!storage_device_ready()) {
            sprintf(buffer1, "{\\\"type\\\":\\\"error\\\",\\\"info\\\":\\\"下载失败：%s\\\"}", __this->playlist.filename);

            dev_mqtt_push_playlist("download", buffer1);
            goto __result_exit;

        }


        if (strlen(meta_uuid_string) > sizeof(__this->playlist.meta_uuid) - 1) {
            memcpy(__this->playlist.meta_uuid, meta_uuid_string, sizeof(__this->playlist.meta_uuid) - 1);
            __this->playlist.meta_uuid[sizeof(__this->playlist.meta_uuid) - 1] = 0;
        } else {
            strcpy(__this->playlist.meta_uuid, meta_uuid_string);
        }



        //创建线程
        __this->playlist.state = WECHAT_MUSIC_DOWNLOADING;
        ret = thread_fork("playlist_download_task", 17, 0x500, 0, 0, playlist_download_task, (void *) &__this->playlist);

        if (ret != OS_NO_ERR) {
            printf("create thread fail ret=%d\n", ret);
            goto __result_exit;

        }
    }

__result_exit:
    free(buffer1);
    json_object_put(new_obj1);

}


/*********************playlist实现***********************************/


/*获取状态*/
static void app_mqtt_cb_user_status(const char *buffer)
{
    char buf[32];
    u8 mac_addr[6];
    dev_mqtt_push_status("online", "设备在线");
    dev_mqtt_push_status("quantity", "20");

    u32 vol = db_select("vol");
    itoa(vol, buf, 10);
    dev_mqtt_push_status("volume", buf);

    memset(buf, 0, 32);
    if (__this->cur.state == WECHAT_MUSIC_PLAYING) {
        strcpy(buf, "play");
    } else if (__this->cur.state == WECHAT_MUSIC_PAUSE) {
        strcpy(buf, "stop");
    }
    dev_mqtt_push_status("play_status", buf);

    memset(buf, 0, 32);

    sprintf(buf, "%d.%d.%d", OTA_MAJOR, OTA_MINOR, OTA_PATCH);


    dev_mqtt_push_status("version", buf);
    dev_mqtt_push_status("play", __this->cur.filename);

    memset(buf, 0, 32);
    void *fd = dev_open("wifi", NULL);

    dev_ioctl(fd, DEV_GET_MAC, (u32)&mac_addr);
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x"
            , mac_addr[0]
            , mac_addr[1]
            , mac_addr[2]
            , mac_addr[3]
            , mac_addr[4]
            , mac_addr[5]);

    dev_mqtt_push_status("mac", buf);


    struct cfg_info info;
    info.mode = NONE_MODE;
    dev_ioctl(fd, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    dev_mqtt_push_status("wifi", info.ssid);
    dev_close(fd);

    memset(buf, 0, 32);


    u32 space = 0;
    struct vfs_partition *part = NULL;
    if (storage_device_ready() == 0) {

        dev_mqtt_push_status("storage", "不在线");
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);
        fget_free_space(CONFIG_ROOT_PATH, &space);
        sprintf(buf, "%dMB/%dMB", space / 1024, part->total_size / 1024);

        dev_mqtt_push_status("storage", buf);

    }

}


/*用于微信语音*/
void dev_mqtt_cb_sys_msg(char *cmd, char *parm)
{
    app_mqtt_cb_sys_speech(parm);
}

void dev_mqtt_cb_user_msg(char *cmd, char *parm)
{
    if (!strcmp(cmd, "play")) {
        app_mqtt_cb_user_play(parm);
    } else if (!strcmp(cmd, "status")) {
        app_mqtt_cb_user_status(parm);
    } else if (!strcmp(cmd, "list")) {
        printf("no support list\n");
    } else if (!strcmp(cmd, "stop")) {
        printf("no support stop\n");
    } else if (!strcmp(cmd, "next")) {
        app_mqtt_cb_user_next(NULL);
    } else if (!strcmp(cmd, "pre")) {
        app_mqtt_cb_user_pre(NULL);
    } else if (!strcmp(cmd, "pause")) {
        JL_wechat_media_audio_pause(NULL);
        app_mqtt_cb_user_pause(NULL);
    } else if (!strcmp(cmd, "continue")) {
        if (__this->cur.state == WECHAT_MUSIC_STOP) {
            JL_wechat_media_audio_play(__this->cur.fileurl);
            __this->cur.state = WECHAT_MUSIC_PLAYING;
            dev_mqtt_push_status("play_status", "play");

        } else if (__this->cur.state == WECHAT_MUSIC_PAUSE) {
            JL_wechat_media_audio_continue(NULL);
            app_mqtt_cb_user_pause(NULL);
        }
    } else if (!strcmp(cmd, "download")) {
        app_mqtt_cb_user_playlist_download(parm);
    } else if (!strcmp(cmd, "update")) {
        app_mqtt_cb_user_update(NULL);
        /* app_mqtt_cb_user_pre(NULL); */
    } else {
        printf("no support \n");
    }
}


void wechat_api_task(void *arg)
{
    int msg[32];
    int err;
    while (1) {
        err = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (err != OS_TASKQ || msg[0] != Q_USER) {
            continue;
        }

        switch (msg[1]) {
        case WECHAT_NEXT_SONG:
            puts("===========WECHAT_NEXT_END\n");
            app_mqtt_cb_user_next(NULL);
            break;
        case WECHAT_PRE_SONG:
            puts("===========WECHAT_PRE_END\n");
            app_mqtt_cb_user_pre(NULL);
            break;
        case WECHAT_PAUSE_SONG:
            puts("===========WECHAT_PAUSE\n");
            app_mqtt_cb_user_pause(NULL);
            break;
        case WECHAT_VOLUME_CHANGE:
            puts("===========WECHAT_VOLUME_CHANGE\n");
            app_mqtt_cb_user_volume_change(NULL);
            break;
        case WECHAT_MEDIA_END:
            puts("===========WECHAT_MEDIA_END\n");
            app_mqtt_cb_user_next(NULL);
            break;
        case WECHAT_MEDIA_STOP:
            puts("===========WECHAT_MEDIA_STOP\n");
            app_mqtt_cb_user_stop(NULL);
            break;
        case WECHAT_KILL_SELF_TASK:
            printf("\nwechat task kill\n");
            return;
        default:
            break;
        }
    }
}
#endif

