
ifeq ($(CPU),AC521x)

export LIBS = \
--start-group \
$(AR_DIR)/rtos.a \
$(AR_DIR)/sd1.a  \
$(AR_DIR)/ascii.a  \
$(AR_DIR)/fs.a  \
$(AR_DIR)/upgrade.a \
$(AR_DIR)/rs_ecc.a \
$(AR_DIR)/server.a  \
$(AR_DIR)/led_ui_server.a  \
$(AR_DIR)/vm.a  \
$(AR_DIR)/audio_server.a  \
$(AR_DIR)/mp3_dec_lib.a \
$(AR_DIR)/m4a_dec_lib.a \
$(AR_DIR)/wav_dec_lib.a \
$(AR_DIR)/wma_dec_lib.a \
$(AR_DIR)/amr_dec_lib.a \
$(AR_DIR)/amr_enc_lib.a \
$(AR_DIR)/speex_enc_lib.a \
$(AR_DIR)/cpu_vm_icache.a  \
--whole-archive \
$(AR_DIR)/cpu.a  \
$(AR_DIR)/dev.a  \
$(AR_DIR)/audio_dev.a  \
$(AR_DIR)/sys.a \
$(AR_DIR)/mm.a  \
--no-whole-archive \
$(AR_DIR)/printf.a \
$(AR_DIR)/lbuf.a  \
$(AR_DIR)/cbuf.a  \

endif


ifeq ($(CONFIG_NET_ENABLE),y)
LIBS += \
$(AR_DIR)/mbedtls.a\
$(AR_DIR)/iperf.a\
$(AR_DIR)/lwip.a\
$(AR_DIR)/common_lib.a\
$(AR_DIR)/sdio_card.a\
$(AR_DIR)/hostapd_and_wpasupplicant.a \
$(AR_DIR)/rtl8189e.a\
$(AR_DIR)/hsm.a\
$(AR_DIR)/http_cli.a\
$(AR_DIR)/http_server.a\
$(AR_DIR)/json.a\
$(AR_DIR)/lib_mqtt.a\
$(AR_DIR)/libduer-device.a\
$(AR_DIR)/network_download.a\
$(AR_DIR)/jieli_net.a\
$(AR_DIR)/libturing.a\
$(AR_DIR)/libmpeg.a\
$(AR_DIR)/libMultSine_Control.a  \
$(AR_DIR)/ai_server.a\
$(AR_DIR)/zliblite.a\
$(AR_DIR)/profile.a\
--whole-archive \
$(AR_DIR)/wifi_dev.a  \
--no-whole-archive \

endif

LIBS += \
include_lib/liba/$(CPU)/libc.a \
include_lib/liba/$(CPU)/libm.a \
--end-group

LIBS += \
/opt/pi32v2/newlib/lib/libcompiler-rt.a \

objs += \
app/debug.o \
app/flash_write_protect.o \
app/sd_speed_test.o \
app/fs_test.o \
app/$(APP_CASE)/app_database.o \
app/$(APP_CASE)/app_main.o \
app/$(APP_CASE)/device_mount.o \
app/$(APP_CASE)/version.o \
app/$(APP_CASE)/mqtt_cmd.o \
app/$(APP_CASE)/turing_mqtt_cmd.o \
app/$(APP_CASE)/app_music.o \
app/$(APP_CASE)/led_ui/led_ui_gr202.o \
app/$(APP_CASE)/upgrade.o \
app/$(APP_CASE)/turing_cmd.o \


ifeq ($(CONFIG_NET_ENABLE),y)
objs += \
app/$(APP_CASE)/wifi_app_task.o \
app/net/wifi-tools/ap_config.o \
app/net/wifi-tools/RTL_PHY_REG_PG.o \
app/net/wifi-tools/rtw_cache.o \
app/net/wifi-tools/uaputl_conf.o \
app/net/wifi-tools/iperf_test.o \
app/net/wifi-tools/voiceprint_cfg.o \

endif


includes += \
-I$(ROOT)/app/$(APP_CASE)/include \
-I$(ROOT)/include_lib/ \
-I$(ROOT)/app \
-I$(ROOT)/app/$(APP_CASE) \
-I$(ROOT)/app/net/include \
-I$(ROOT)/include_lib/net \
-I$(ROOT)/include_lib/net/server \
-I$(ROOT)/include_lib/system/cpu/$(CPU) \
-I$(ROOT)/include_lib/net/wifi_module \
-I$(ROOT)/include_lib/net/device/ \
-I$(ROOT)/include_lib/net/device/eth \
-I$(ROOT)/include_lib/net/hostapdandwpa_supplicant \
-I$(ROOT)/include_lib/net/lwip/src/include \
-I$(ROOT)/include_lib/net/lwip/port \
-I$(ROOT)/include_lib/net/lwip/app \
-I$(ROOT)/include_lib/net/lwip/app/ctp \
-I$(ROOT)/include_lib/net/lwip/app/cdp \



