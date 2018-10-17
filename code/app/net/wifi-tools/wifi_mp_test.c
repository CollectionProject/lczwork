#include <string.h>
#include "rtl_mp.h"
#include "os/os_compat.h"
#include "FCCStruct.h"

extern u16 CRC16(void *ptr, u32  len);
extern int spec_uart_recv(char *buf, u32 len);
extern int spec_uart_send(char *buf, u32 len);


typedef struct __FCC_DATA {
    char mark[2];
    unsigned short  _id;
    unsigned short  data_len;
    unsigned char  *data_buf;
    unsigned short  crc;
} FCC_DATA;

typedef struct _MP_UART_DATA {
    char buf[64];
    unsigned int len;
} S_MP_UART_DATA;

const char *antenna_x_tab[] = {
    "a",
    "b",
    "ab",
};
const char *tx_rate_tab[] = {
    "1M",
    "2M",
    "5.5M",
    "6M",
    "9M",
    "11M",
    "12M",
    "18M",
    "24M",
    "36M",
    "48M",
    "54M",
    "HTMCS0",
    "HTMCS1",
    "HTMCS2",
    "HTMCS3",
    "HTMCS4",
    "HTMCS5",
    "HTMCS6",
    "HTMCS7",
};
static void wifi_mp_test_deal(S_MP_UART_DATA *data)
{
    static unsigned char mac_adr[6] __attribute__((aligned(32)));
    static unsigned int rx_ok __attribute__((aligned(32)));
    static unsigned char txpower __attribute__((aligned(32)));

    char *buf;
    unsigned int len;
    FCC_DATA *fcc = NULL;
    unsigned int crc_check_len;
    unsigned char rssi;
    buf = data->buf;
    len = data->len;
    if (len < 2) {
        return;
    }

    if (!(buf[0] == 'J' && buf[1] == 'L')) {
        return;
    }
    fcc = (FCC_DATA *)calloc(1, sizeof(FCC_DATA));
    if (fcc == NULL) {
        goto __exit;
    }
    memcpy((unsigned char *) & (fcc->mark),     buf,  sizeof(fcc->mark));
    memcpy((unsigned char *) & (fcc->_id),      buf + sizeof(fcc->mark), sizeof(fcc->_id));
    memcpy((unsigned char *) & (fcc->data_len), buf + sizeof(fcc->mark) + sizeof(fcc->_id), sizeof(fcc->data_len));
    memcpy((unsigned char *) & (fcc->crc),      buf + sizeof(fcc->mark) + sizeof(fcc->_id) + sizeof(fcc->data_len) + fcc->data_len, sizeof(fcc->crc));

    fcc->data_buf = (unsigned char *)calloc(1, fcc->data_len);
    if (fcc->data_buf == NULL) {
        goto __exit;
    }

    memcpy(fcc->data_buf, buf + sizeof(fcc->mark) + sizeof(fcc->_id) + sizeof(fcc->data_len), fcc->data_len);
//    fcc.data_buf = buf + sizeof(fcc.mark)+sizeof(fcc._id)+sizeof(fcc.data_len);
    printf("fcc.crc:%x\n", fcc->crc);
    printf("fcc.data_len:%x\n", fcc->data_len);
    printf("fcc._id:%x\n", fcc->_id);
    put_buf(fcc->data_buf, fcc->data_len);
    crc_check_len = sizeof(fcc->mark) + sizeof(fcc->_id) + sizeof(fcc->data_len) + fcc->data_len;
    if (fcc->crc != CRC16(buf, crc_check_len)) {
        printf("crc check err\r");
        goto __exit;
    }

    printf("data buf:\n");
    put_buf(fcc->data_buf, fcc->data_len);
    printf("\n");

    switch (fcc->_id) {
    case STRUCT_ID_FCC_CPTX:
        if (fcc->data_len != sizeof(FCC_CPTX)) {
            printf("1 data size err\r");
        } else {

            FCC_CPTX *cptx = (FCC_CPTX *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", cptx->mp_channel, cptx->bandwidth, cptx->short_gi);
            printf("\r\n- - - antenna_x is %s,pathx_txpower is %d,mp_rate is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[cptx->antenna_x], cptx->pathx_txpower, (unsigned char *)tx_rate_tab[cptx->mp_rate], cptx->tx_durtime);
            Continuous_Packet_Tx_testing(cptx->mp_channel,
                                         cptx->bandwidth,
                                         cptx->short_gi,
                                         (unsigned char *)antenna_x_tab[cptx->antenna_x],
                                         cptx->pathx_txpower,
                                         (unsigned char *)tx_rate_tab[cptx->mp_rate],
                                         cptx->tx_durtime);
        }

        break;

    case STRUCT_ID_FCC_CPTX_COUNT:

        if (fcc->data_len != sizeof(FCC_CPTX_COUNT)) {
            printf("2 data size err\r");
        } else {
            FCC_CPTX_COUNT *cptx_c = (FCC_CPTX_COUNT *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", cptx_c->mp_channel, cptx_c->bandwidth, cptx_c->short_gi);
            printf("\r\n- - - antenna_x is %s,pathx_txpower is %d,mp_rate is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[cptx_c->antenna_x], cptx_c->pathx_txpower, (unsigned char *)tx_rate_tab[cptx_c->mp_rate], cptx_c->npackets);
            Count_Packet_Tx_testing(cptx_c->mp_channel,
                                    cptx_c->bandwidth,
                                    cptx_c->short_gi,
                                    (unsigned char *)antenna_x_tab[cptx_c->antenna_x],
                                    cptx_c->pathx_txpower,
                                    (unsigned char *)tx_rate_tab[cptx_c->mp_rate],
                                    cptx_c->npackets);
        }

        break;

    case STRUCT_ID_FCC_CS:
        if (fcc->data_len != sizeof(FCC_CS)) {
            printf("3 data size err\r");
        } else {
            FCC_CS *_cs = (FCC_CS *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", _cs->mp_channel, _cs->bandwidth, _cs->short_gi);
            printf("\r\n- - - antenna_x is %s,pathx_txpower is %d,mp_rate is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[_cs->antenna_x], _cs->pathx_txpower, (unsigned char *)tx_rate_tab[_cs->mp_rate], _cs->tx_durtime);
            Carrier_suppression_testing(_cs->mp_channel,
                                        _cs->bandwidth,
                                        _cs->short_gi,
                                        (unsigned char *)antenna_x_tab[_cs->antenna_x],
                                        _cs->pathx_txpower,
                                        (unsigned char *)tx_rate_tab[_cs->mp_rate],
                                        _cs->tx_durtime);
        }
        break;

    case STRUCT_ID_FCC_CTX:
        if (fcc->data_len != sizeof(FCC_CTX)) {
            printf("0 data size err\r");
        } else {
            FCC_CTX *ctx = (FCC_CTX *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", ctx->mp_channel, ctx->bandwidth, ctx->short_gi);
            printf("\r\n- - - antenna_x is %s,pathx_txpower is %d,mp_rate is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[ctx->antenna_x], ctx->pathx_txpower, (unsigned char *)tx_rate_tab[ctx->mp_rate], ctx->tx_durtime);
            Continuous_Tx_testing(ctx->mp_channel,
                                  ctx->bandwidth,
                                  ctx->short_gi,
                                  (unsigned char *)antenna_x_tab[ctx->antenna_x],
                                  ctx->pathx_txpower,
                                  (unsigned char *)tx_rate_tab[ctx->mp_rate],
                                  ctx->tx_durtime);
        }

        break;

    case STRUCT_ID_FCC_MAC_GET:
        if (fcc->data_len != sizeof(FCC_MAC_GET)) {
            printf("6 data size err\r");
        } else {
            FCC_MAC_GET *_mac = (FCC_MAC_GET *)fcc->data_buf;
            put_buf(fcc->data_buf, fcc->data_len);
            printf("_mac->mac_get:%d\n", _mac->mac_get);
            if (_mac->mac_get) {
                mp_efuse_get_mac(mac_adr);
                printf("mac adress:\n");
                put_buf(mac_adr, 6);
            } else {
                memset(mac_adr, 0, 6);
            }
//                char rx_str[128];
//                sprintf(rx_str, "%x%x%x%x%x%x", mac_adr[0], mac_adr[1], mac_adr[2], mac_adr[3], mac_adr[4], mac_adr[5]);
//                printf("rx_str=%s\n", rx_str);
//                   spec_uart_send(mac_adr, 6);
        }
        break;

    case STRUCT_ID_FCC_RX:
        if (fcc->data_len != sizeof(FCC_RX)) {
            printf("5 data size err\r");
        } else {
            FCC_RX *_ar = (FCC_RX *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", _ar->mp_channel, _ar->bandwidth, _ar->short_gi);
            printf("\r\n- - - antenna_x is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[_ar->antenna_x], _ar->rx_durtime);
            Air_Rx_testing(_ar->mp_channel,
                           _ar->bandwidth,
                           _ar->short_gi,
                           (unsigned char *)antenna_x_tab[_ar->antenna_x],
                           _ar->rx_durtime);
        }

//            char rx_str[64];
        mp_query(NULL, &rx_ok);
//            sprintf(rx_str, "%d", rx_ok);
//            printf("rx_str=%s\n", rx_str);

        spec_uart_send(&rx_ok, 4);
        break;

    case STRUCT_ID_FCC_STT:
        if (fcc->data_len != sizeof(FCC_STT)) {
            printf("4 data size err\r");
        } else {
            FCC_STT *stt = (FCC_STT *)fcc->data_buf;
            printf("\r\n\r\n- - - channel is %d,bandwidth is %d,short_gi is %d - - -\r\n", stt->mp_channel, stt->bandwidth, stt->short_gi);
            printf("\r\n- - - antenna_x is %s,pathx_txpower is %d,mp_rate is %s,tx_durtime is %d - - -\r\n\r\n", (unsigned char *)antenna_x_tab[stt->antenna_x], stt->pathx_txpower, (unsigned char *)tx_rate_tab[stt->mp_rate], stt->tx_durtime);
            Single_Tone_Tx_testing(stt->mp_channel,
                                   stt->bandwidth,
                                   stt->short_gi,
                                   (unsigned char *)antenna_x_tab[stt->antenna_x],
                                   stt->pathx_txpower,
                                   (unsigned char *)tx_rate_tab[stt->mp_rate],
                                   stt->tx_durtime);
        }
        break;

        /*case STRUCT_ID_FCC_TXPOWER:
            if(fcc->data_len != sizeof(FCC_TXPOWER))
            {
                printf("6 data size err\r");
            }
            else
            {
                FCC_TXPOWER *_txpow = (FCC_TXPOWER*)fcc->data_buf;
                if(_txpow->mp_txpower)
                {
                    printf("_txpow->antenna_x:%d\r", _txpow->antenna_x);
                    mp_get_tx_power(_txpow->antenna_x, &txpower);
                }
                else
                {
                    txpower = 0;
                }
                spec_uart_send(&txpower, 1);
            }
            break;*/

#if 0
    case STRUCT_ID_FCC_RSSI:
        if (fcc->data_len != sizeof(FCC_RSSI)) {
            printf("6 data size err\r");
        } else {
            FCC_RSSI *_rssi = (FCC_RSSI *)fcc->data_buf;
            if (_rssi->mp_rssi) {
                extern unsigned char get_rx_signal(void);
                rssi = get_rx_signal();
                printf("rssi: %x\n", rssi);
            } else {
                rssi = 0;
            }
            spec_uart_send(&rssi, 1);
        }
        break;
#endif

    default:
        break;
    }


__exit:
    if (fcc) {
        if (fcc->data_buf) {
            free(fcc->data_buf);
        }
        free(fcc);
    }
    free(data);
}

void wifi_mp_uart_test(void)
{
    S_MP_UART_DATA *uart_data;
    int wifi_mp_test_pid = 0;

    while (1) {
        uart_data = calloc(1, sizeof(S_MP_UART_DATA));
        uart_data->len = spec_uart_recv(uart_data->buf, sizeof(uart_data->buf));
        printf("\n-------------------------------------------------\n");
//        spec_uart_send(uart_data.buf, uart_data.len);
        put_buf(uart_data->buf, uart_data->len);

        set_mp_test_status(1);
        thread_kill(&wifi_mp_test_pid, 0);
        set_mp_test_status(0);
        printf("\n-------------uart addr is 0x%X-------------\n", &uart_data);

        thread_fork("wifi_mp_test_task", 21, 2000, 0, &wifi_mp_test_pid, wifi_mp_test_deal, (void *)uart_data);
    }
    printf("mp_uart_test over!....\n");
}

