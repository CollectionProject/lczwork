

ifeq ($(CONFIG_NET_ENABLE),y)
objs += \
app/wifi/wifi_app_task.o \
app/wifi/network_mssdp.o \
app/wifi/wifi-tools/ap_config.o \
app/wifi/wifi-tools/RTL_PHY_REG_PG.o \
app/wifi/wifi-tools/rtw_cache.o \
app/wifi/wifi-tools/uaputl_conf.o \
app/wifi/wifi-tools/iperf_test.o \


includes += \
-I$(ROOT)/app/wifi/include \
-I$(ROOT)/include_lib/net  \
-I$(ROOT)/include_lib/net/wifi_module \
-I$(ROOT)/include_lib/net/lwip/port/ \
-I$(ROOT)/include_lib/net/lwip/app/ \
-I$(ROOT)/include_lib/net/lwip/src/include  \
-I$(ROOT)/include_lib/net/lwip/app/ctp \
-I$(ROOT)/include_lib/net/lwip/app/cdp \
-I$(ROOT)/include_lib/net/hostapdandwpa_supplicant \
-I$(ROOT)/include_lib/net/wifi_module/ \


endif



