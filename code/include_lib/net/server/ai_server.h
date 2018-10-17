#ifndef AI_SERVER_H
#define AI_SERVER_H

#include "generic/typedef.h"
#include "server/audio_server.h"
#include "stdarg.h"

enum {
    AI_SERVER_EVENT_URL,
    AI_SERVER_EVENT_CONNECTED,
    AI_SERVER_EVENT_DISCONNECTED,
    AI_SERVER_EVENT_CONTINUE,
    AI_SERVER_EVENT_PAUSE,
};


#define AI_REQ_LISTEN       0x10
#define AI_REQ_EVENT        0x12


enum {
    AI_STAT_CONNECTED,
    AI_STAT_DISCONNECTED,
};


enum ai_server_event {
    AI_EVENT_SPEAK_END     = 0x01,
    AI_EVENT_MEDIA_END     = 0x02,
    AI_EVENT_PLAY_PAUSE    = 0x03,
    AI_EVENT_PREVIOUS_SONG = 0x04,
    AI_EVENT_NEXT_SONG     = 0x05,
    AI_EVENT_VOLUME_CHANGE = 0X06,
    AI_EVENT_VOLUME_INCR   = 0x07,
    AI_EVENT_VOLUME_DECR   = 0x08,
    AI_EVENT_VOLUME_MUTE   = 0x09,
    AI_EVENT_RECORD_START  = 0x0a,
    AI_EVENT_RECORD_STOP   = 0x0c,
    AI_EVENT_VOICE_MODE    = 0x0d,
    AI_EVENT_PLAY_TIME     = 0x0e,
    AI_EVENT_MEDIA_STOP    = 0x0f,
    AI_EVENT_QUIT    	   = 0xff,
};

enum {
    AI_REQ_CONNECT,
    AI_LISTEN_START,
    AI_LISTEN_STOP,
};

struct ai_sdk_api {
    const char *name;
    int (*connect)();
    int (*state_check)();
    int (*do_event)(int event, ...);
    int (*disconnect)();
};

struct ai_listen {
    int cmd;
};

struct ai_event {
    int event;
    char volume;
    u16 progress;
    const char *ai_name;
};


union ai_req {
    struct ai_listen lis;
    struct ai_event evt;

};


int ai_server_event_url(const struct ai_sdk_api *, const char *url, int);

int ai_server_do_event(int event, ...);


extern const struct ai_sdk_api ai_sdk_api_begin[];
extern const struct ai_sdk_api ai_sdk_api_end[];


#define REGISTER_AI_SDK(name) \
    const struct ai_sdk_api name sec(.ai_sdk)


#define list_for_each_ai_sdk(p) \
    for (index = 0, p = ai_sdk_api_begin; p < ai_sdk_api_end; p++,index++)






#endif
