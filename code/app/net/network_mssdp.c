#include "mssdp/mssdp.h"
#include "fs/fs.h"
#include "app_config.h"

#include "system/event.h"
#if defined CONFIG_NET_SERVER || defined CONFIG_NET_CLIENT
static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    if (type == MSSDP_SEARCH_MSG) {
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
    } else if (type == MSSDP_NOTIFY_MSG) {
        printf("sssss\n");
//TODO
#if 1
        struct sys_event e;
        e.arg = "dhcp_srv";
        e.type = SYS_DEVICE_EVENT;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        sys_event_notify(&e);
#endif


        mssdp_uninit();

    }
}
#else

#define JSON_DOC "aaa"

static char json_buffer[1024];

const char *__attribute__((weak))get_dev_desc_doc()
{
    int ret;
    FILE *fd = fopen("mnt/spiflash/res/dev_desc.txt", "r");

    if (fd == NULL) {
        printf("%s~~~~~~~~%d   open fail\n", __func__, __LINE__);
        return NULL;
    }

    memset(json_buffer, 0, 1024);
    ret = fread(fd, json_buffer, 1024);
    if (ret <= 0) {
        printf("%s~~~~~~~~%d   read fail\n", __func__, __LINE__);
        return NULL;
    }

    fclose(fd);

    return json_buffer;
}

static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    if (type == MSSDP_SEARCH_MSG) {
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
    }
}
#endif

void network_mssdp_init(void)
{
    puts("mssdp run \n");
    mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", 3889, network_ssdp_cb, NULL);

#if defined  CONFIG_NET_SERVER
    mssdp_set_search_msg("xyz", 30); //主动3秒
#endif
#if defined  CONFIG_NET_CLIENT
    mssdp_set_notify_msg("xy", 5); //被动30秒
#endif
#if (!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER)
    mssdp_set_notify_msg((const char *)get_dev_desc_doc(), 60);
#endif
}

void network_mssdp_uninit(void)
{
    mssdp_uninit();
}



