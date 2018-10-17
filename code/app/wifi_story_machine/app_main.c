#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "mem_leak_test.h"



u32 spin_lock_cnt[2] = {0};

int upgrade_detect(const char *sdcard_name);


/*任务列表 */
const struct task_info task_info_table[] = {
    {"led_ui_server",       30,     2048,   64    },
    {"init",                30,     1024,   256   },
    {"app_core",            15,     2048,   1024  },
    {"sys_event",           30,     1024,   0     },
    {"audio_server",        16,     1024,   256   },
    {"wechat_server",       17,     1024,   64    },
    {"audio_decoder",       30,     1024,   64    },
    {"audio_encoder",       14,     1024,   64    },
    {"speex_encoder",       10,     1024,   64    },
    {"wechat_task",         18,     2048,   64    },
    {"amr_encoder",         16,     1024,   64    },
    {"usb_server",          20,     2048,   128   },
    {"khubd",               25,     1024,   32    },

    {"upgrade_server",      21,     1024,   32    },
    {"upgrade_ui",          21,     1024,   32    },
    {"upgrade_core",        20,     1024,   32    },

    {"auto_test",			15,		1024,	1024  },
    {"fs_test",			    15,		1024,	0     },
    {"ai_server",			15,		1024,	64     },

    {0, 0},
};


#ifdef CONFIG_UI_ENABLE
/*
 * 开机动画播放完毕
 */
static void animation_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

    server_close(ui);

    /*
     * 显示完开机画面后更新配置文件,避免效果调节过度导致开机图片偏色
     */
    void *imd = dev_open("imd", NULL);
    if (imd) {
        dev_ioctl(imd, IMD_SET_COLOR_CFG, (u32)"scr_auto.bin"); /* 更新配置文件  */
        dev_close(imd);
    }

    /*
     *按键消息使能
     */
    {
        sys_key_event_enable();
        sys_touch_event_enable();//使能触摸事件
    }
}

/*
 * 关机动画播放完毕, 关闭电源
 */
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u32 park_en;

    if (ui) {
        server_close(ui);
    }

    sys_power_set_port_wakeup("wkup_usb", 1);
    sys_power_poweroff(0);
}
#endif


static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_MODE:
            break;
        default:
            return false;
        }
        break;
    case KEY_EVENT_LONG:
        break;
    default:
        return false;
    }

    return true;
}

extern u8 get_usb_in_status();
static int main_dev_event_handler(struct sys_event *event)
{
    struct intent it;
    struct application *app;

    init_intent(&it);
    app = get_current_app();

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_OUT:
        break;
    case DEVICE_EVENT_CHANGE:
        break;
    }
    return 0;
}


/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;
    case SYS_NET_EVENT:
        break;
    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}

#ifdef RTOS_STACK_CHECK_ENABLE
static void rtos_stack_check_func(void *p)
{
    char *pWriteBuffer = malloc(2048);
    if (!pWriteBuffer) {
        return;
    }
    extern void vTaskList(char *pcWriteBuffer);
    vTaskList(pWriteBuffer);
    printf(" \n\ntask_name          task_state priority stack task_num\n%s\n", pWriteBuffer);
    free(pWriteBuffer);

    malloc_stats();
    malloc_debug_show();

}
#endif

extern void set_duer_package_info(const char *product,
                                  const char *batch,
                                  const char *os_name,
                                  const char *developer,
                                  const char *os_version,
                                  const char *staged_version);

/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;
    int err;

    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL);
    }

    mount_sd_to_fs(SDX_DEV);

#ifdef CONFIG_DISPLAY_ENABLE
    lcd_backlight_ctrl(true);
#endif

    /*err = upgrade_detect(SDX_DEV);
    if (!err) {
        return;
    }*/

    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = ANI_UI_VERSION;

    struct server *ui = server_open("ui_server", &style);
    if (ui) {
        union uireq req;

        req.show.id = ANI_ID_PAGE_POWER_ON;
        server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                             animation_play_end, ui);
    }
#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif

    sys_power_auto_shutdown_start(db_select("aff") * 60);
    sys_power_charger_off_shutdown(PWR_DELAY_INFINITE);


    init_intent(&it);
    it.name = "app_music";
    it.action = ACTION_MUSIC_PLAY_MAIN;
    start_app(&it);

#ifdef RTOS_STACK_CHECK_ENABLE
    sys_timer_add(NULL, rtos_stack_check_func, 60 * 1000);
#endif

#ifdef CONFIG_DUER_SDK_ENABLE
    char firmware_version[16];
    set_duer_package_info("WIFI_STORY", "12", "FreeRTOS", "xueyong", "0.0.0.0", "0.0.0.0");	//设置版本号
    if (db_select_buffer(9, firmware_version, 16) > 0) {
        if ((u8)firmware_version[0] < 16) {
            firmware_version[firmware_version[0] + 1] = 0;
            set_duer_package_info("WIFI_STORY", "12", "FreeRTOS", "xueyong", firmware_version + 1, firmware_version + 1);	//设置版本号
            printf("firmware_version : %s", firmware_version + 1);
        }
    }
#endif
}



