#include "system/includes.h"

#include "res.h"
#include "app_database.h"
#include "app_config.h"


#define CN_PA   ((0xA9BE << 16) | ('A' << 8)  | ('B' << 0))
#define CN_PB   (('C'    << 24) | ('D' << 16) | ('E' << 8) | ('F' << 0))


/*
 * app配置项表
 * 参数1: 配置项名字
 * 参数2: 配置项需要多少个bit存储
 * 参数3: 配置项的默认值
 */
static const struct db_table app_config_table[] = {
    {"kvo",     1,      1 },                         // 按键音开关
    {"vol",     8,      30},                         // 音量
    {"aff",     8,      5},                         // 自动关机时间, 单位分钟

    {"dac",     16,     0x55aa},                         // dac_trim
};



int app_set_config(struct intent *it, const struct app_cfg *cfg, int size)
{
    int i;

    printf("app_set_config: %s, %d\n", it->data, it->exdata);

    for (i = 0; i < size; i++) {
        if (!strcmp(it->data, cfg[i].table)) {
            if (cfg[i].set) {
                int err = cfg[i].set(it->exdata);
                if (err) {
                    return err;
                }
            }
            db_update(cfg[i].table, it->exdata);
            return 0;
        }
    }

    return -EINVAL;
}





static int app_config_init()
{
    int err;

#if defined CONFIG_DATABASE_2_RTC
    err = db_create("rtc");
    ASSERT(err == 0, "open device rtc faild\n");
#elif defined CONFIG_DATABASE_2_FLASH
    err = db_create("vm");
    ASSERT(err == 0, "open device vm faild\n");
#else
#error "undefine database device"
#endif

    return db_create_table(app_config_table, ARRAY_SIZE(app_config_table));
}
__initcall(app_config_init);


