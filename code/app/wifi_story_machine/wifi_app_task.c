
#include "server/wifi_connect.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "wifi_ext.h"
#include "system/init.h"
#include "lwip.h"
#include "server/net_server.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "action.h"
#include "system/timer.h"
#include "http/http_server.h"
#include "asm/debug.h"
#include "net_config.h"
#include "app_config.h"
#include "device/wifi_dev.h"
#include "server/network_mssdp.h"
#include "http/http_cli.h"
#include "system/timer.h"
#include "database.h"
#include "wechat/wechat_core.h"
#include "wechat/ota_update.h"
#include "net/wifi-tools/voiceprint_cfg.h"

#define WIFI_APP_TASK_NAME "wifi_app_task"

enum WIFI_APP_MSG_CODE {
    WIFI_MSG_TICK_1_SEC,
    WIFI_MSG_SMP_CFG_START,
    WIFI_MSG_SMP_CFG_STOP,
    WIFI_MSG_SMP_CFG_COMPLETED,
    WIFI_MSG_SMP_CFG_TIMEOUT,
    WIFI_MSG_STA_SCAN_COMPLETED,
    WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_MSG_STA_DISCONNECTED,
};

static u8 mac_addr[6];
static char g_ssid[32];
static char g_pwd[64];
static void *wifi_dev;
static u8 _net_dhcp_ready = 0;
static struct airkiss_result {
    struct smp_cfg_result result;
    char scan_ssid_found;
} airkiss_result;

static struct voiceprint_result {
    char rand_str[8];
} voiceprint_result;

int net_dhcp_ready()
{
    return (_net_dhcp_ready);
}

static void wifi_app_timer_func(void *p)
{
    os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_TICK_1_SEC);
}

static void wifi_taskq_post(int msg)
{
    int ret = 0;
    u8 retry = 0;

    do {
        ret = os_taskq_post(WIFI_APP_TASK_NAME, 1, msg);
        if (ret == OS_NO_ERR) {
            break;
        }
        msleep(50);
        retry++;
    } while (retry < 5);

    if (ret != OS_NO_ERR) {
        printf("post msg %d to wifi_app_task fail !!! \n", msg);
    }
}

void wifi_smp_connect(char *ssid, char *pwd, void *rand_str)
{
    struct cfg_info info = {0};
    if (ssid) {
        strcpy(voiceprint_result.rand_str, rand_str);
        info.mode = STA_MODE;
        info.ssid = ssid;
        info.pwd = pwd;
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
        dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
    } else {

        dev_ioctl(wifi_dev, DEV_GET_WIFI_SMP_RESULT, (u32)&info);

        if (info.smp_cfg.type == AIRKISS_SMP_CFG) {
            printf("\r\n AIRKISS INFO, SSID = %s, PWD = %s, ssid_crc = 0x%x, ran_val = 0x%x \r\n", info.smp_cfg.ssid, info.smp_cfg.passphrase, info.smp_cfg.ssid_crc, info.smp_cfg.random_val);
            airkiss_result.result.type = AIRKISS_SMP_CFG ;
            airkiss_result.result.ssid_crc = info.smp_cfg.ssid_crc;
            airkiss_result.result.random_val = info.smp_cfg.random_val;
            strcpy(airkiss_result.result.ssid, info.smp_cfg.ssid);
            strcpy(airkiss_result.result.passphrase, info.smp_cfg.passphrase);
            dev_ioctl(wifi_dev, DEV_NET_SCANF, 0);
        } else {
            info.mode = STA_MODE;
            info.ssid = info.smp_cfg.ssid;
            info.pwd = info.smp_cfg.passphrase;
            dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
            dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
        }
    }
}

static void airkiss_ssid_check(void)
{
    u32 i;
    struct cfg_info info = {0};
    if (airkiss_result.result.type != AIRKISS_SMP_CFG ||  airkiss_result.scan_ssid_found) {
        return;
    }

    dev_ioctl(wifi_dev, DEV_GET_STA_SSID_INFO, (u32)&info);

    for (i = 0; i < info.sta_ssid_num; i++) {
        if (!strncmp(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid, strlen(airkiss_result.result.ssid))) {
CHECK_AIRKISS_SSID_CRC:
            ;
            extern u8 airkiss_calcrc_bytes(u8 * p, unsigned int num_of_bytes);
            if (airkiss_result.result.ssid_crc == airkiss_calcrc_bytes((u8 *)info.sta_ssid_info[i].ssid, strlen(info.sta_ssid_info[i].ssid))) {
                printf("find airkiss ssid = [%s]\r\n", info.sta_ssid_info[i].ssid);
                strcpy(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid);
                airkiss_result.scan_ssid_found = 1;

                info.mode = STA_MODE;
                info.ssid = airkiss_result.result.ssid;
                info.pwd = airkiss_result.result.passphrase;
                dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);

                dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);

                return;
            }
        } else {
            /*goto CHECK_AIRKISS_SSID_CRC;*/
        }
    }
    printf("cannot found airkiss ssid[%s] !!! \n\n", airkiss_result.result.ssid);

}


static void voiceprint_broadcast(void)
{
    if (voiceprint_result.rand_str[0]) {

#define SEND_VOICE_PRINT_MSG		"https://robot.jieliapp.com/wx/wechat/network/config/voice/success?wifirand=%s"

        int ret = 0;
        char url[1024];

        http_body_obj http_body_buf;
        httpcli_ctx ctx;

        sprintf(url, SEND_VOICE_PRINT_MSG, voiceprint_result.rand_str);

        memset(&http_body_buf, 0x0, sizeof(http_body_obj));
        memset(&ctx, 0x0, sizeof(ctx));
        http_body_buf.recv_len = 0;
        http_body_buf.buf_len = 5 * 1024;
        http_body_buf.buf_count = 1;
        http_body_buf.p = (char *) malloc(http_body_buf.buf_len * sizeof(char));
        ctx.url = url;
        ctx.priv = &http_body_buf;
        ctx.connection = "close";
        ret = httpscli_get(&ctx);
        if (ret == HERROR_OK) {
            ret = 0;
        } else {
            puts("voiceprint_broadcast fail!\n");
            ret = -1;
        }

        if (http_body_buf.p) {
            free(http_body_buf.p);
        }

        voiceprint_result.rand_str[0] = 0;
    }
}

static void airkiss_broadcast(void)
{
    int i, ret;
    int onOff = 1;
    int sock;
    struct sockaddr_in dest_addr;

    if (airkiss_result.result.type != AIRKISS_SMP_CFG) {
        return;
    }


    puts("airkiss_broadcast random_val \n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        printf("%s %d->Error in socket()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = 0;
    ret = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret == -1) {
        printf("%s %d->Error in bind()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                     (char *)&onOff, sizeof(onOff));
    if (ret == -1) {
        printf("%s %d->Error in setsockopt() SO_BROADCAST\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    inet_pton(AF_INET, "255.255.255.255", &dest_addr.sin_addr.s_addr);
    dest_addr.sin_port = htons(10000);

    for (i = 0; i < 8; i++) {
        ret =	sendto(sock, (unsigned char *)&airkiss_result.result.random_val, 1, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
        if (ret == -1) {
            printf("%s %d->Error in sendto\n", __FUNCTION__, __LINE__);
        }
        msleep(20);
    }

    memset(&airkiss_result, 0, sizeof(airkiss_result));

EXIT:
    if (sock != -1) {
        closesocket(sock);
    }
    return;
}

static int network_user_callback(void *network_ctx, enum NETWORK_EVENT state, void *priv)
{
    struct cfg_info info;

    switch (state) {

    case WIFI_EVENT_MODULE_INIT:

#if 0
        gpio_set_hd(IO_PORTG_00, 0);
        gpio_set_hd(IO_PORTG_01, 0);
        gpio_set_hd(IO_PORTG_02, 0);
        gpio_set_hd(IO_PORTG_03, 0);
        gpio_set_hd(IO_PORTG_04, 0);
        gpio_set_hd(IO_PORTG_05, 0);

#endif


//wifi module port seting
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_CS, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_WKUP, (u32)&info);

        msleep(100);

        info.mode = AP_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SET_DEFAULT_MODE, (u32)&info);
        break;

    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");
        info.mode = SMP_CFG_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);
        break;
    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;

    case WIFI_EVENT_AP_START:
        puts("|network_user_callback->WIFI_EVENT_AP_START\n");
        break;
    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");
#if 0//8801要if 1，8189不能开，开了切换不了模式
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;
    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
#if 0
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;
    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        wifi_taskq_post(WIFI_MSG_STA_DISCONNECTED);
        break;
    case WIFI_EVENT_STA_SCAN_COMPLETED:
        /* if(wpa_supplicant_get_state() != STA_WPA_CONNECT_COMPLETED) */
        /* { */
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        wifi_taskq_post(WIFI_MSG_STA_SCAN_COMPLETED);
        /* } */
        break;
    case WIFI_EVENT_STA_CONNECT_SUCC:
        dev_ioctl(wifi_dev, DEV_GET_WIFI_CHANNEL, (u32)&info);
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", info.sta_channel);
        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        break;
    case WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        wifi_taskq_post(WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC);
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;
    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;
    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_START:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_START\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_START);
        break;
    case WIFI_EVENT_SMP_CFG_STOP:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_STOP\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_STOP);
        break;
    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_TIMEOUT);
        break;
    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_COMPLETED);
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;
    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;
    case WIFI_EVENT_AP_ON_ASSOC:
        ;
        struct eth_addr *hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_ASSOC hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_DISCONNECTED hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
        break;
    default:
        break;
    }

    return 0;
}


static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 1,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 192,
        .WIRELESS_GATEWAY1  = 168,
        .WIRELESS_GATEWAY2  = 1,
        .WIRELESS_GATEWAY3  = 1,

        .SERVER_IPADDR1  = 192,
        .SERVER_IPADDR2  = 168,
        .SERVER_IPADDR3  = 1,
        .SERVER_IPADDR4  = 1,

        .CLIENT_IPADDR1  = 192,
        .CLIENT_IPADDR2  = 168,
        .CLIENT_IPADDR3  = 1,
        .CLIENT_IPADDR4  = 2,

        .SUB_NET_MASK1   = 255,
        .SUB_NET_MASK2   = 255,
        .SUB_NET_MASK3   = 255,
        .SUB_NET_MASK4   = 0,
    };

    struct cfg_info info;
    info.__lan_setting_info = &lan_setting_info;
    dev_ioctl(wifi_dev, DEV_SET_LAN_SETTING, (u32)&info);
}


void net_app_init(void)
{

//网络测试工具，使用iperf
    extern void iperf_test(void);
    iperf_test();
}


void net_app_uninit(void)
{
}

void wifi_on(void)
{
    dev_ioctl(wifi_dev, DEV_NETWORK_START, 0);
    net_app_init();
}


void wifi_off(void)
{
    net_app_uninit();
    dev_ioctl(wifi_dev, DEV_NETWORK_STOP, 0);
}

void wifi_enter_smp_cfg_mode(void)
{
    struct cfg_info info = {0};
    info.timeout = 100;
#ifdef CONFIG_DUER_WECHAT_ENABLE
    dev_ioctl(wifi_dev, DEV_SET_SMP_AIRKISS_AES_ON_OFF, 1);
#endif
    dev_ioctl(wifi_dev, DEV_SMP_MODE, 0);
    dev_ioctl(wifi_dev, DEV_SET_SMP_CONFIG_TIMEOUT_SEC, (u32)&info);
}

int get_wifi_is_smp_mode(void)
{
    struct cfg_info info = {0};
    info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    return info.mode == SMP_CFG_MODE;
}

void wifi_return_sta_mode(void)
{
    struct cfg_info info = {0};
    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}

void wifi_app_task(void *priv)
{
    int msg[32];
    int res;
    u32 timehdl = 0;
    struct cfg_info info = {0};

    //if (!db_select("wfo")) {
    //    puts("wifi onoff : OFF \r\n");
    //    return;
    // }


//wifi app start
    wifi_dev = dev_open("wifi", NULL);
    if (!wifi_dev) {
        printf(">>>>>>> wifi dev open err !!!! <<<<<<<<\r\n");
        return ;
    }

    info.cb = network_user_callback;
    info.net_priv = NULL;
    dev_ioctl(wifi_dev, DEV_SET_CB_FUNC, (u32)&info);

#if 0
    printf("\n >>>> DEV_SET_WIFI_POWER_SAVE<<<<   \n");
    dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER_SAVE, 0);//打开就启用低功耗模式, 只有STA模式才有用
#endif

#if 1
    printf("\n >>>> DEV_SET_WIFI_TX_PWR_BY_RATE<<<   \n");

    info.tx_pwr_lmt_enable = 0;//  解除WIFI发送功率限制
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_LMT_ENABLE, (u32)&info);
    info.tx_pwr_by_rate = 1;// 设置WIFI根据不同datarate打不同power
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_BY_RATE, (u32)&info);
#endif


    wifi_set_lan_setting_info();

    wifi_on();

    sys_timer_add(NULL, wifi_app_timer_func, 1000);

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));

        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case WIFI_MSG_TICK_1_SEC:

                    if (time_lapse(&timehdl, 60 * 1000)) {
                        /*升级检查，并升级*/
                        /* ota_update("wifi_story","device",0,1,0); */

                        /* malloc_stats(); */
//                malloc_debug_show();
                        if (wifi_module_is_init()) {

//                stats_display();
                            printf("WIFI U= %d KB/s, D= %d KB/s\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024);
#if 0
                            if (get_cur_wifi_info()->mode == STA_MODE) {
                                get_rx_signal();
                            }
#endif
                        }
                    }

                    break;

                case WIFI_MSG_SMP_CFG_START:
                    memset(&airkiss_result, 0, sizeof(airkiss_result));
                    voiceprint_cfg_start();
                    break;
                case WIFI_MSG_SMP_CFG_STOP:
                    voiceprint_cfg_stop();
                    break;

                case WIFI_MSG_SMP_CFG_COMPLETED:
                    wifi_smp_connect(NULL, NULL, NULL);
                    break;
                case WIFI_MSG_SMP_CFG_TIMEOUT: {
                    struct sys_event evt;
                    evt.arg = "net";
                    evt.type = SYS_NET_EVENT;
                    evt.u.net.event = NET_EVENT_SMP_CFG_TIMEOUT;
                    sys_event_notify(&evt);
                }
                break;

                case WIFI_MSG_STA_SCAN_COMPLETED:
                    airkiss_ssid_check();
                    break;
                case WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC:
                    airkiss_broadcast();
                    voiceprint_broadcast();
                    _net_dhcp_ready = 1;
                    {

                        struct sys_event evt;
                        evt.arg = "net";
                        evt.type = SYS_NET_EVENT;
                        evt.u.net.event = NET_EVENT_CONNECTED;
                        sys_event_notify(&evt);
                    }
                    break;
                case WIFI_MSG_STA_DISCONNECTED:
                    _net_dhcp_ready = 0;
                    {
                        struct sys_event evt;
                        evt.arg = "net";
                        evt.type = SYS_NET_EVENT;
                        evt.u.net.event = NET_EVENT_DISCONNECTED;
                        sys_event_notify(&evt);
                    }
                    break;

                default :
                    break;
                }
                break;
            default:
                break;
            }
            break;
        }
    }

}

int wireless_net_init(void)//主要是create wifi 线程的
{
    puts("wifi early init \n\n\n\n\n\n");
    return thread_fork(WIFI_APP_TASK_NAME, 10, 2048, 64, 0, wifi_app_task, NULL);
}

#if defined CONFIG_WIFI_ENABLE
late_initcall(wireless_net_init);
#endif

static struct cfg_info wifi_info;
char *get_wifi_ssid(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.ssid;
}


char *get_wifi_pwd(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.pwd;
}


#if 0
void sdio_recv_pkg_irq(void)
{
    static u32 thdll, count222;
    int ret22;
    ret22 = time_lapse(&thdll, 1000);
    if (ret22) {
        /* printf("sdio_recv_cnt = %d,  %d \r\n", ret22, count222); */
        count222 = 0;
    }
    ++count222;
}
#endif
