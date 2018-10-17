#include "app_config.h"

#ifdef CONFIG_NET_ENABLE

#include "net/net_api.h"
#include "system/includes.h"

#include "action.h"
//#include "ftpserver/stupid-ftpd.h"
//#include "device/wifi_dev.h"
//#include "server/rt_stream_pkg.h"
//#include "server/network_mssdp.h"
//


//#include "eth/eth_phy.h"


void *dev = NULL ;

#if 0
static void wifi_set_lan_setting_info(void)
{

    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 3,

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
        .SERVER_IPADDR4  = 3,

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
    dev_ioctl(dev, DEV_SET_LAN_SETTING, (u32)&info);

}
#endif

static void tcp_connect_test(void *arg)
{
    int fd;
    struct sockaddr_in dest_addr;
    struct sockaddr_in remote;
    int ret = 0;

    puts("connect init\n");
    fd = socket(AF_INET, SOCK_STREAM, 0);

    char *buf1 = malloc(44 * 1460);
    remote.sin_family = AF_INET;
    //remote.sin_addr.s_addr = inet_addr("172.16.23.44");//htonl(INADDR_ANY);
    remote.sin_addr.s_addr = inet_addr("192.168.1.100");//htonl(INADDR_ANY);
    remote.sin_port = htons(22222);



    ret = connect(fd, (struct sockaddr *)&remote, sizeof(remote));
    if (ret < 0) {
        puts("connect fail\n\n\n");
    }

    send(fd, "exit", 4, 0);



    puts("connect success\n\n\n");


}





static void dhcp_start_complete(int err)
{
    if (err == 0) {
        tcp_connect_test(NULL);
    } else {
        log_e("dhcp_start_faild err=%d\n", err);
    }
}

static void network_start_complete(int err)
{
    struct net_config_info info;

    if (err == 0) {
        log_d("network_start_complete\n");

        info.dhcp_enable = true;
        info.set_default = true;
        info.local_host_name = "jieli_wifi";

        info.cfg.wifi.mode = ETHDEV_WIFI_STA_MODE;
        info.cfg.wifi.ssid = "FAST_105";
        info.cfg.wifi.password = "caozhilong";

        netapi_icfg_restart("wifi", &info, dhcp_start_complete);
    }

}

void Init_LwIP(u8_t is_wireless, u8_t dhcp);
void net_task(void *arg)
{

    /*char mac_addr[] = {0x12, 0x23, 0x22, 0x44, 0x55, 0x66};*/

    /*dev = dev_open("eth0", NULL);
    extern const struct eth_platform_data net_phy_data;
    ethmac_setup((struct eth_platform_data  *)&net_phy_data);
    Init_LwIP(0, 1);
    os_time_dly(6000 / 10);
    tcp_connect_test(NULL);*/

    struct net_config_info info;

#if 0
    info.dhcp_enable = true;
    info.set_default = true;
    info.local_host_name = "jieli_host";

    //netapi_icfg_set("eth0", &info);
    //netapi_icfg_restart("eth0", dhcp_start_complete);

    info.cfg.wifi.mode = ETHDEV_WIFI_STA_MODE;
    info.cfg.wifi.ssid = "FAST_105";
    info.cfg.wifi.password = "caozhilong";

    netapi_icfg_set("wifi", &info);
    netapi_icfg_restart("wifi", dhcp_start_complete);
#else
    info.dhcp_enable = false;
    info.set_default = true;
    info.local_host_name = "jieli_host";
    IP4_ADDR(&info.ipaddr, 192, 168, 1, 1);
    IP4_ADDR(&info.mask, 255, 255, 255, 0);
    IP4_ADDR(&info.gw, 192, 168, 1, 1);

    info.cfg.wifi.mode = ETHDEV_WIFI_AP_MODE;
    info.cfg.wifi.ssid = "CPYZ";
    info.cfg.wifi.password = "12345678";

    netapi_icfg_restart("wifi", &info, network_start_complete);

#endif


    //网速测试
    /*extern void iperf_test(void);
    iperf_test();*/



    int msg[32];


#if 0
    //后拉数据来源服务
    extern int pull_video_recv_init();
    pull_video_recv_init();
#endif
    while (1) {
        os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        /*os_time_dly(10);*/
    }
}

static int wire_net_init(void)//主要是有线网卡线程的
{
    //thread_fork("net_task", 20, 0x1000, 0, 0, net_task, NULL);
    os_task_create(net_task, NULL, 20, 0x1000, 256,  "net_task");
    return 0;
}

late_initcall(wire_net_init);

#endif
