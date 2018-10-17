
#ifndef APP_MUSIC
#define APP_MUSIC

#include "system/includes.h"
#include "app_config.h"

struct music_menu_sta {
    u8 aaa;
};



struct music_play_hdl {
    struct server *audio_server;
    struct music_menu_sta menu_status;
};










#endif

