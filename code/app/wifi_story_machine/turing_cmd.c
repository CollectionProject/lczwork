#include "os/os_compat.h"
#include "turing/turing.h"

#ifdef CONFIG_TURING_SDK_ENABLE

static int turing_cb(int code, struct json_object *func)
{
    struct json_object *new = NULL;
    struct json_object *url = NULL;
    struct json_object *isPlay = NULL;
    int ret = 0;

    switch (code) {
    case TURING_EVENT_FUN_SLEEP_CTL:
    case TURING_EVENT_FUN_DANCE:
    case TURING_EVENT_FUN_SPORT_CTL:
    case TURING_EVENT_FUN_ALARM_CTL:
        break;
    case TURING_EVENT_FUN_VOL_CTL:
        if (json_object_object_get_ex(func, "operate", &new)) {
            int is = json_object_get_int(new);
            if (json_object_object_get_ex(new, "arg", &new)) {
                ret = json_object_get_int(new);
                if (is) {
                    //增大音量
                } else {
                    //减小音量
                }
            }
        }
        break;
    case TURING_EVENT_FUN_SCREEN_LIGHT:
        if (json_object_object_get_ex(func, "operate", &new)) {
            int is = json_object_get_int(new);
            if (json_object_object_get_ex(new, "arg", &new)) {
                ret = json_object_get_int(new);
                if (is) {
                    //增大亮度
                } else {
                    //减小亮度
                }
            }
        }
        break;
    case TURING_EVENT_FUN_PLAY_MUSIC:
    case TURING_EVENT_FUN_PLAY_STORY:
        if (json_object_object_get_ex(func, "operate", &new)) {
            ret = json_object_get_int(new);
        }
        if (json_object_object_get_ex(func, "isPlay", &isPlay)) {
            int is = json_object_get_int(isPlay);
            if (is == 1) {
                if (json_object_object_get_ex(func, "url", &url)) {
                    set_turing_media_url(json_object_get_string(url));
                }
            }
        }
        break;
    case TURING_EVENT_FUN_ANIMAL_SOUND:
    case TURING_EVENT_FUN_GUESS_SOUND:
        if (json_object_object_get_ex(func, "url", &url)) {
            set_turing_media_url(json_object_get_string(url));
        }
        break;
    default:
        break;
    }

    return 0;
}
#define APIKEY "3d442018244f477d8fc84b2f05d6571c"
#define DEVICE_ID "aiAA001122334483"
#define SECRET_KEY "58AF23xUN47Y4Tt4"
void init_turing_para(void)
{
    struct turing_para para = {0};

    strcpy(para.user_id, DEVICE_ID);
    strcpy(para.aes_key, SECRET_KEY);
    strcpy(para.api_key, APIKEY);

    para.asr = ASR_SPEEX;
    para.tts = TTS_MP3_16;
    para.flag = OUTPUT_ARS_TTS_TXT;
    para.real_time = STREAM_IDENTIFY;
    para.encode = NORMAL_ENCODE;
    para.type = SMARTCHAT;
    para.speed = 5;
    para.pitch = 5;
    para.volume = 5;
    para.tone = 14;	//1, 14, 21
    para.asr_lan = 0;
    para.tts_lan = 0;

    set_turing_para(&para, 5000, turing_cb);
}

#endif
