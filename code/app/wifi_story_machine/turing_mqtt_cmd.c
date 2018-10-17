#include "http/http_cli.h"
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
#include "event.h"


#ifdef CONFIG_TURING_SDK_ENABLE
#define RECV_DATA_SIZE	1024

struct wechat_html_file_info {
    char filename[128];
    char filetime[16];
    char fileurl[1024];
    char meta_uuid[128];
    char album_id[32];
    u32 meta_sn;
    u32 state;
    u32 message_type;
};

struct tl_iot_device_state {
    int vol;
    int battery;
    int sfree;
    int stotal;
    int shake;
    int power;
    int bln;
    int play;
    int charging;
    int lbi;
    int tcard;
};



enum {
    WECHAT_MUSIC_PLAYING = 0x1,
    WECHAT_MUSIC_PAUSE,
    WECHAT_MUSIC_STOP,
    WECHAT_MUSIC_DOWNLOADING,
    WECHAT_MUSIC_DOWNLOAD_FLINSH,

};

enum {
    MQTT_MESSAGE_CHAT = 0x0,
    MQTT_MESSAGE_AUDIO,
    MQTT_MESSAGE_CONTROL,
    MQTT_MESSAGE_NOTIFY
};

enum {
    MQTT_MESSAGE_MUSIC_PLAY = 0x1,
    MQTT_MESSAGE_MUSIC_STOP,
};

enum {
    CONCROL_VOL_DOWN = 0x0,
    CONCROL_VOL_UP,
    CONCROL_LED_CLOSE,
    CONCROL_LED_OPEN,
    CONCROL_LOW_POWER_OPEN,
    CONCROL_LOW_POWER_CLOSE,
    CONCROL_STROAGE_FORMAT,
    CONCROL_DEFAULT,
    CONCROL_SET_CLOCK,
    CONCROL_CALL_PHONE,
    CONCROL_SLEEP_TIME,
    CONCROL_SET_UP,
    CONCROL_SET_CLOSE_PLAYING,
};






struct __wechat_html_status {
    struct wechat_html_file_info cur;
    struct wechat_html_file_info playlist;
    struct tl_iot_device_state dev_state;
};

static struct __wechat_html_status wechat_html_status;

#define __this (&wechat_html_status)


extern void turing_wechat_media_audio_play(const char *url);
extern void turing_wechat_media_audio_continue(const char *url);
extern void turing_wechat_media_audio_pause(const char *url);
extern int turing_wechat_state_noitfy(u8 type, const char *status_buffer) ;
extern char *itoa(int num, char *str, int radix);
extern int atoi(const char *__nptr);
extern int turing_wechat_server_event_url(const char *url, int event);
extern int wechat_music_dec_volume(int step);
extern int get_app_music_volume(void);
extern int turing_wechat_next_song(char *title, char *id);
extern int turing_wechat_pre_song(char *title, char *id);
extern int turing_wechat_pause_song(char *title, char *id, u8 status);


int mqtt_message_chat(json_object *message)
{
    json_object *url = NULL;

    if (!json_object_object_get_ex(message, "url", &url)) {
        return -1;
    }

    const char *url_string = json_object_get_string(url);

    turing_wechat_server_event_url(url_string, WECHAT_SERVER_SPEECH_URL_EVENT);

    return 0;

}
int mqtt_message_audio(json_object *message)
{
    json_object *url = NULL;
    json_object *operate = NULL;
    json_object *album_id = NULL;
    json_object *meta_sn = NULL;
    json_object *name = NULL;


    if (!json_object_object_get_ex(message, "arg", &name)) {
        return -1;
    }

    const char *name_string = json_object_get_string(name);

    if (strlen(name_string) > sizeof(__this->cur.filename) - 1) {
        memcpy(__this->cur.filename, name_string, sizeof(__this->cur.filename) - 1);
        __this->cur.filename[sizeof(__this->cur.filename) - 1] = 0;
    } else {
        strcpy(__this->cur.filename, name_string);
    }


    if (!json_object_object_get_ex(message, "url", &url)) {
        return -1;
    }


    const char *url_string = json_object_get_string(url);



    if (strlen(url_string) > sizeof(__this->cur.fileurl) - 1) {
        memcpy(__this->cur.fileurl, url_string, sizeof(__this->cur.fileurl) - 1);
        __this->cur.fileurl[sizeof(__this->cur.fileurl) - 1] = 0;
    } else {
        strcpy(__this->cur.fileurl, url_string);
    }


    if (!json_object_object_get_ex(message, "mediaId", &album_id)) {
        return -1;
    }

    int id = json_object_get_int(album_id);

    if (!json_object_object_get_ex(message, "operate", &operate)) {
        return -1;
    }


    int music_state = json_object_get_int(operate);

    //to do something
    if (music_state == MQTT_MESSAGE_MUSIC_PLAY && id != atoi(__this->cur.album_id)) {
        turing_wechat_media_audio_play(__this->cur.fileurl);
        __this->cur.state = WECHAT_MUSIC_PLAYING;
        sprintf(__this->cur.album_id, "%d", id);
        printf("\n\n\nplay new  music\n\n\n");
    } else {
        turing_wechat_media_audio_pause(NULL);
        /* __this->cur.state = WECHAT_MUSIC_PAUSE; */
        printf("\n\n\nplay old  music\n\n\n");

    }

    return 0;

}


int mqtt_message_control(json_object *message)
{
    json_object *operate = NULL;
    json_object *arg = NULL;

    if (!json_object_object_get_ex(message, "operate", &operate)) {
        return -1;
    }

    int state = json_object_get_int(operate);

    switch (state) {
    case CONCROL_VOL_DOWN:
    case CONCROL_VOL_UP:
        printf("CONCROL_SET_VOL\n");
        if (!json_object_object_get_ex(message, "arg", &arg)) {
            return -1;
        }
        int vol = json_object_get_int(arg);
        wechat_music_dec_volume(vol);

        break;
    case CONCROL_LED_CLOSE:
        printf("CONCROL_LED_CLOSE\n");

        struct sys_event evt;
        evt.type = SYS_KEY_EVENT;
        evt.u.key.event = KEY_EVENT_CLICK;
        evt.u.key.value = KEY_MODE;
        sys_event_notify(&evt);

        break;
    case CONCROL_LED_OPEN:
        printf("CONCROL_LED_OPEN\n");

        struct sys_event evt1;
        evt1.type = SYS_KEY_EVENT;
        evt1.u.key.event = KEY_EVENT_CLICK;
        evt1.u.key.value = KEY_MODE;
        sys_event_notify(&evt1);


        break;
    case CONCROL_LOW_POWER_CLOSE:
        printf("CONCROL_LOW_POWER_CLOSE\n");
        break;
    case CONCROL_LOW_POWER_OPEN:
        printf("CONCROL_LOW_POWER_OPEN\n");
        break;
    case CONCROL_STROAGE_FORMAT:
        printf("CONCROL_STROAGE_FORMAT\n");
        /* storage_device_format(); */
        break;
    case CONCROL_DEFAULT:
        printf("CONCROL_DEFAULT\n");
        /* db_reset(); */
        break;
    default:
        printf("no support\n");
        break;

    }





    return 0;

}

int mqtt_message_noitfy(json_object *message)
{
    printf("no support\n");
    return 0;
}

int mqtt_init_state_noitfy(json_object *message)
{
#define SSS "\"vol\": %d,\"battery\": %d,\"sfree\": %d,\"stotal\": %d,\"shake\": %d,\"power\": %d,\"bln\": %d,\"play\": %d,\"charging\": %d,\"lbi\": %d,\"tcard\": %d"

    int buf[512];

    /* __this->dev_state.vol = db_select("vol"); */

    __this->dev_state.vol = get_app_music_volume();

    u32 space = 0;
    struct vfs_partition *part = NULL;
    if (storage_device_ready() == 0) {

        /* dev_mqtt_push_status("storage", "不在线"); */
        __this->dev_state.tcard = 0;
        __this->dev_state.sfree = 0;
        __this->dev_state.stotal = 0;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);
        fget_free_space(CONFIG_ROOT_PATH, &space);

        __this->dev_state.tcard = 1;
        __this->dev_state.sfree = space;
        __this->dev_state.stotal = part->total_size;

    }

    {
        //没实现


        __this->dev_state.battery  = 100;
        __this->dev_state.shake = 1;//意义不明

        __this->dev_state.power = 0; //是否低电
        __this->dev_state.bln = 1; //是否开启灯

        __this->dev_state.play = 0;

        __this->dev_state.charging = 0;//是否正在充电中

        __this->dev_state.lbi  = 0;//是否开启低电提示

    }

    sprintf(buf, SSS
            , __this->dev_state.vol
            , __this->dev_state.battery
            , __this->dev_state.sfree
            , __this->dev_state.stotal
            , __this->dev_state.shake
            , __this->dev_state.power
            , __this->dev_state.bln
            , __this->dev_state.play
            , __this->dev_state.charging
            , __this->dev_state.lbi
            , __this->dev_state.tcard);

    turing_wechat_state_noitfy(0, buf);
    return 0;

}



void all_case(char *parm)
{

    json_object *new_obj1 = NULL;
    json_object *type = NULL;
    json_object *message = NULL;
    new_obj1 = json_tokener_parse(parm);
    if (!json_object_object_get_ex(new_obj1, "type", &type)) {
        goto __result_exit;
    }
    __this->cur.message_type = json_object_get_int(type);
    if (!json_object_object_get_ex(new_obj1, "message", &message)) {
        goto __result_exit;
    }



    switch (__this->cur.message_type) {

    case MQTT_MESSAGE_CHAT:
        mqtt_message_chat(message);
        break;
    case MQTT_MESSAGE_AUDIO:
        mqtt_message_audio(message);
        break;
    case MQTT_MESSAGE_CONTROL:
        mqtt_message_control(message);
        break;
    case MQTT_MESSAGE_NOTIFY:
        mqtt_message_noitfy(message);
        break;
    default:
        break;

    }

__result_exit:
    json_object_put(new_obj1);


}



void dev_mqtt_cb_user_msg(char *cmd, char *parm)
{
    if (!strcmp(cmd, "turing")) {
        all_case(parm);
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
            turing_wechat_next_song(__this->cur.filename, __this->cur.album_id);
            __this->cur.state = WECHAT_MUSIC_PLAYING;
            break;
        case WECHAT_PRE_SONG:
            puts("===========WECHAT_PRE_END\n");
            turing_wechat_pre_song(__this->cur.filename, __this->cur.album_id);
            __this->cur.state = WECHAT_MUSIC_PLAYING;
            break;
        case WECHAT_PAUSE_SONG:
            puts("===========WECHAT_PAUSE\n");
            if (__this->cur.state  == WECHAT_MUSIC_PAUSE) {
                turing_wechat_pause_song(__this->cur.filename, __this->cur.album_id, 1);
                __this->cur.state = WECHAT_MUSIC_PLAYING;
            } else if (__this->cur.state == WECHAT_MUSIC_PLAYING) {
                turing_wechat_pause_song(__this->cur.filename, __this->cur.album_id, 2);
                __this->cur.state = WECHAT_MUSIC_PAUSE;
            }

            break;
        case WECHAT_VOLUME_CHANGE:
            puts("===========WECHAT_VOLUME_CHANGE\n");
            char buf[32];
            sprintf(buf, "\"vol\":%d", get_app_music_volume());
            turing_wechat_state_noitfy(0, buf);
            break;
        case WECHAT_MEDIA_END:
            puts("===========WECHAT_MEDIA_END\n");

            turing_wechat_next_song(__this->cur.filename, __this->cur.album_id);
            __this->cur.state = WECHAT_MUSIC_PLAYING;
            break;
        case WECHAT_MEDIA_STOP:
            puts("===========WECHAT_MEDIA_STOP\n");
            break;
        case WECHAT_SEND_INIT_STATE:
            puts("===========WECHAT_SEND_INIT_STATE\n");
            mqtt_init_state_noitfy(NULL);
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


