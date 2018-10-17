#ifndef __USB_INC_H__
#define __USB_INC_H__
#include "asm/cpu.h"

#if 0
#define HUSB_EP5_TADR           (*(volatile u32 *)(ls_husb_base + 0x0f*4))      //26bit write only;
#define HUSB_EP5_RADR           (*(volatile u32 *)(ls_husb_base + 0x10*4))      //26bit write only;

#define HUSB_EP6_TADR           (*(volatile u32 *)(ls_husb_base + 0x11*4))      //26bit write only;
#define HUSB_EP6_RADR           (*(volatile u32 *)(ls_husb_base + 0x12*4))      //26bit write only;

#endif
#define H_EP5TXMAXP     (*(volatile u16 *)(husb_base + 0x150))
#define H_EP5TXCSR      (*(volatile u16 *)(husb_base + 0x152))
#define H_EP5RXMAXP     (*(volatile u16 *)(husb_base + 0x154))
#define H_EP5RXCSR      (*(volatile u16 *)(husb_base + 0x156))
#define H_EP5RXCOUNT    (*(volatile u16 *)(husb_base + 0x158))
#define H_EP5TXTYPE     (*(volatile u8  *)(husb_base + 0x15a))
#define H_EP5TXINTERVAL (*(volatile u8  *)(husb_base + 0x15b))
#define H_EP5RXTYPE     (*(volatile u8  *)(husb_base + 0x15c))
#define H_EP5RXINTERVAL (*(volatile u8  *)(husb_base + 0x15d))
#define H_EP5FIFOSIZE   (*(volatile u8  *)(husb_base + 0x15f))




#define H_EP6TXMAXP     (*(volatile u16 *)(husb_base + 0x160))
#define H_EP6TXCSR      (*(volatile u16 *)(husb_base + 0x162))
#define H_EP6RXMAXP     (*(volatile u16 *)(husb_base + 0x164))
#define H_EP6RXCSR      (*(volatile u16 *)(husb_base + 0x166))
#define H_EP6RXCOUNT    (*(volatile u16 *)(husb_base + 0x168))
#define H_EP6TXTYPE     (*(volatile u8  *)(husb_base + 0x16a))
#define H_EP6TXINTERVAL (*(volatile u8  *)(husb_base + 0x16b))
#define H_EP6RXTYPE     (*(volatile u8  *)(husb_base + 0x16c))
#define H_EP6RXINTERVAL (*(volatile u8  *)(husb_base + 0x16d))
#define H_EP6FIFOSIZE   (*(volatile u8  *)(husb_base + 0x16f))



#define H_EP0_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x080))
#define H_EP0_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x082))
#define H_EP0_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x083))
#define H_EP0_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x084))
#define H_EP0_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x086))
#define H_EP0_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x087))

#define H_EP1_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x088))
#define H_EP1_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x08a))
#define H_EP1_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x08b))
#define H_EP1_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x08c))
#define H_EP1_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x08e))
#define H_EP1_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x08f))

#define H_EP2_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x090))
#define H_EP2_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x092))
#define H_EP2_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x093))
#define H_EP2_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x094))
#define H_EP2_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x096))
#define H_EP2_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x097))

#define H_EP3_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x098))
#define H_EP3_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x09a))
#define H_EP3_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x09b))
#define H_EP3_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x09c))
#define H_EP3_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x09e))
#define H_EP3_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x09f))

#define H_EP4_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0a0))

#define H_EP4_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x0a2))
#define H_EP4_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x0a3))
#define H_EP4_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0a4))
#define H_EP4_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x0a6))
#define H_EP4_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x0a7))

#define H_EP5_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0a8))

#define H_EP5_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x0aa))
#define H_EP5_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x0ab))
#define H_EP5_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0ac))
#define H_EP5_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x0ae))
#define H_EP5_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x0af))

#define H_EP6_TXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0b0))

#define H_EP6_TXHUBADDR    (*(volatile u8  *)(husb_base + 0x0b2))
#define H_EP6_TXHUBPORT    (*(volatile u8  *)(husb_base + 0x0b3))
#define H_EP6_RXFUNCADDR   (*(volatile u8  *)(husb_base + 0x0b4))
#define H_EP6_RXHUBADDR    (*(volatile u8  *)(husb_base + 0x0b6))
#define H_EP6_RXHUBPORT    (*(volatile u8  *)(husb_base + 0x0b7))

//...........  High Speed USB .....................

#define HUSB_FADDR                        &H_FADDR
#define HUSB_POWER                        &H_POWER
#define HUSB_INTRTX                       &H_INTRTX
#define HUSB_INTRRX                       &H_INTRRX
#define HUSB_INTRTXE                      &H_INTRTXE
#define HUSB_INTRRXE                      &H_INTRRXE
#define HUSB_INTRUSB                      &H_INTRUSB
#define HUSB_INTRUSBE                     &H_INTRUSBE
#define HUSB_FRAME                        &H_FRAME
#define HUSB_INDEX                        &H_INDEX
#define HUSB_TESTMODE                     &H_TESTMODE

#define HUSB_FIFO0                        &H_FIFO0
#define HUSB_FIFO1                        &H_FIFO1
#define HUSB_FIFO2                        &H_FIFO2
#define HUSB_FIFO3                        &H_FIFO3
#define HUSB_FIFO4                        &H_FIFO4
#define HUSB_DEVCTL                       &H_DEVCTL
#define HUSB_TYPE0                        &H_TYPE0

#define HUSB_EP0_TXFUNCADDR                 &H_EP0_TXFUNCADDR
#define HUSB_EP0_TXHUBADDR                  &H_EP0_TXHUBADDR
#define HUSB_EP0_TXHUBPORT                  &H_EP0_TXHUBPORT
#define HUSB_EP0_RXFUNCADDR                 &H_EP0_RXFUNCADDR
#define HUSB_EP0_RXHUBADDR                  &H_EP0_RXHUBADDR
#define HUSB_EP0_RXHUBPORT                  &H_EP0_RXHUBPORT

#define HUSB_EP1_TXFUNCADDR                 &H_EP1_TXFUNCADDR
#define HUSB_EP1_TXHUBADDR                  &H_EP1_TXHUBADDR
#define HUSB_EP1_TXHUBPORT                  &H_EP1_TXHUBPORT
#define HUSB_EP1_RXFUNCADDR                 &H_EP1_RXFUNCADDR
#define HUSB_EP1_RXHUBADDR                  &H_EP1_RXHUBADDR
#define HUSB_EP1_RXHUBPORT                  &H_EP1_RXHUBPORT

#define HUSB_EP2_TXFUNCADDR                 &H_EP2_TXFUNCADDR
#define HUSB_EP2_TXHUBADDR                  &H_EP2_TXHUBADDR
#define HUSB_EP2_TXHUBPORT                  &H_EP2_TXHUBPORT
#define HUSB_EP2_RXFUNCADDR                 &H_EP2_RXFUNCADDR
#define HUSB_EP2_RXHUBADDR                  &H_EP2_RXHUBADDR
#define HUSB_EP2_RXHUBPORT                  &H_EP2_RXHUBPORT

#define HUSB_EP3_TXFUNCADDR                 &H_EP3_TXFUNCADDR
#define HUSB_EP3_TXHUBADDR                  &H_EP3_TXHUBADDR
#define HUSB_EP3_TXHUBPORT                  &H_EP3_TXHUBPORT
#define HUSB_EP3_RXFUNCADDR                 &H_EP3_RXFUNCADDR
#define HUSB_EP3_RXHUBADDR                  &H_EP3_RXHUBADDR
#define HUSB_EP3_RXHUBPORT                  &H_EP3_RXHUBPORT

#define HUSB_EP4_TXFUNCADDR                 &H_EP4_TXFUNCADDR
#define HUSB_EP4_TXHUBADDR                  &H_EP4_TXHUBADDR
#define HUSB_EP4_TXHUBPORT                  &H_EP4_TXHUBPORT
#define HUSB_EP4_RXFUNCADDR                 &H_EP4_RXFUNCADDR
#define HUSB_EP4_RXHUBADDR                  &H_EP4_RXHUBADDR
#define HUSB_EP4_RXHUBPORT                  &H_EP4_RXHUBPORT

#define HUSB_EP5_TXFUNCADDR                 &H_EP5_TXFUNCADDR
#define HUSB_EP5_TXHUBADDR                  &H_EP5_TXHUBADDR
#define HUSB_EP5_TXHUBPORT                  &H_EP5_TXHUBPORT
#define HUSB_EP5_RXFUNCADDR                 &H_EP5_RXFUNCADDR
#define HUSB_EP5_RXHUBADDR                  &H_EP5_RXHUBADDR
#define HUSB_EP5_RXHUBPORT                  &H_EP5_RXHUBPORT

#define HUSB_EP6_TXFUNCADDR                 &H_EP6_TXFUNCADDR
#define HUSB_EP6_TXHUBADDR                  &H_EP6_TXHUBADDR
#define HUSB_EP6_TXHUBPORT                  &H_EP6_TXHUBPORT
#define HUSB_EP6_RXFUNCADDR                 &H_EP6_RXFUNCADDR
#define HUSB_EP6_RXHUBADDR                  &H_EP6_RXHUBADDR
#define HUSB_EP6_RXHUBPORT                  &H_EP6_RXHUBPORT

#define HUSB_CSR0                           &H_CSR0
#define HUSB_COUNT0                         &H_COUNT0
#define HUSB_NAKLIMIT0                      &H_NAKLIMIT0
#define HUSB_CFGDATA                        &H_CFGDATA

#define H_EP0TXMAXP            (*(volatile u16 *)(husb_base + 0x100))
#define HUSB_EP0TXMAXP                    &H_EP0TXMAXP

#define HUSB_EP1TXMAXP                    &H_EP1TXMAXP
#define HUSB_EP1TXCSR                     &H_EP1TXCSR
#define HUSB_EP1RXMAXP                    &H_EP1RXMAXP
#define HUSB_EP1RXCSR                     &H_EP1RXCSR
#define HUSB_EP1RXCOUNT                   &H_EP1RXCOUNT
#define HUSB_EP1TXTYPE                    &H_EP1TXTYPE
#define HUSB_EP1TXINTERVAL                &H_EP1TXINTERVAL
#define HUSB_EP1RXTYPE                    &H_EP1RXTYPE
#define HUSB_EP1RXINTERVAL                &H_EP1RXINTERVAL
#define HUSB_EP1FIFOSIZE                  &H_EP1FIFOSIZE

#define HUSB_EP2TXMAXP                    &H_EP2TXMAXP
#define HUSB_EP2TXCSR                     &H_EP2TXCSR
#define HUSB_EP2RXMAXP                    &H_EP2RXMAXP
#define HUSB_EP2RXCSR                     &H_EP2RXCSR
#define HUSB_EP2RXCOUNT                   &H_EP2RXCOUNT
#define HUSB_EP2TXTYPE                    &H_EP2TXTYPE
#define HUSB_EP2TXINTERVAL                &H_EP2TXINTERVAL
#define HUSB_EP2RXTYPE                    &H_EP2RXTYPE
#define HUSB_EP2RXINTERVAL                &H_EP2RXINTERVAL
#define HUSB_EP2FIFOSIZE                  &H_EP2FIFOSIZE

#define HUSB_EP3TXMAXP                    &H_EP3TXMAXP
#define HUSB_EP3TXCSR                     &H_EP3TXCSR
#define HUSB_EP3RXMAXP                    &H_EP3RXMAXP
#define HUSB_EP3RXCSR                     &H_EP3RXCSR
#define HUSB_EP3RXCOUNT                   &H_EP3RXCOUNT
#define HUSB_EP3TXTYPE                    &H_EP3TXTYPE
#define HUSB_EP3TXINTERVAL                &H_EP3TXINTERVAL
#define HUSB_EP3RXTYPE                    &H_EP3RXTYPE
#define HUSB_EP3RXINTERVAL                &H_EP3RXINTERVAL
#define HUSB_EP3FIFOSIZE                  &H_EP3FIFOSIZE

#define HUSB_EP4TXMAXP                    &H_EP4TXMAXP
#define HUSB_EP4TXCSR                     &H_EP4TXCSR
#define HUSB_EP4RXMAXP                    &H_EP4RXMAXP
#define HUSB_EP4RXCSR                     &H_EP4RXCSR
#define HUSB_EP4RXCOUNT                   &H_EP4RXCOUNT
#define HUSB_EP4TXTYPE                    &H_EP4TXTYPE
#define HUSB_EP4TXINTERVAL                &H_EP4TXINTERVAL
#define HUSB_EP4RXTYPE                    &H_EP4RXTYPE
#define HUSB_EP4RXINTERVAL                &H_EP4RXINTERVAL
#define HUSB_EP4FIFOSIZE                  &H_EP4FIFOSIZE

#define HUSB_EP5TXMAXP                    &H_EP5TXMAXP
#define HUSB_EP5TXCSR                     &H_EP5TXCSR
#define HUSB_EP5RXMAXP                    &H_EP5RXMAXP
#define HUSB_EP5RXCSR                     &H_EP5RXCSR
#define HUSB_EP5RXCOUNT                   &H_EP5RXCOUNT
#define HUSB_EP5TXTYPE                    &H_EP5TXTYPE
#define HUSB_EP5TXINTERVAL                &H_EP5TXINTERVAL
#define HUSB_EP5RXTYPE                    &H_EP5RXTYPE
#define HUSB_EP5RXINTERVAL                &H_EP5RXINTERVAL
#define HUSB_EP5FIFOSIZE                  &H_EP5FIFOSIZE

#define HUSB_EP6TXMAXP                    &H_EP6TXMAXP
#define HUSB_EP6TXCSR                     &H_EP6TXCSR
#define HUSB_EP6RXMAXP                    &H_EP6RXMAXP
#define HUSB_EP6RXCSR                     &H_EP6RXCSR
#define HUSB_EP6RXCOUNT                   &H_EP6RXCOUNT
#define HUSB_EP6TXTYPE                    &H_EP6TXTYPE
#define HUSB_EP6TXINTERVAL                &H_EP6TXINTERVAL
#define HUSB_EP6RXTYPE                    &H_EP6RXTYPE
#define HUSB_EP6RXINTERVAL                &H_EP6RXINTERVAL
#define HUSB_EP6FIFOSIZE                  &H_EP6FIFOSIZE

#define HUSB_TX_DPKTDIS                   &H_TX_DPBUFDIS
#define HUSB_C_T_UCH                      &H_C_T_UCH

enum {
    HUSB = 0,
    MAX_USB_PHY_NUM,
};

enum {
    USB_EP0 = 0,
    USB_EP1,
    USB_EP2,
    USB_EP3,
    USB_EP4,
    USB_EP5,
    USB_EP6,
    USB_EP_MAX,
};

struct musb_regs {
    volatile u8 faddr;
    volatile u8 power;
    volatile u16 intrtx;
    volatile u16 intrrx;
    volatile u16 intrtxe;
    volatile u16 intrrxe;
    volatile u8 intrusb;
    volatile u8 intrusbe;
    volatile u16 frame;
    volatile u8 index;
    volatile u8 testmode;
} ;

struct sie_regs {
    volatile u32 sie_con;
    volatile u32 ep0_cnt;
    volatile u32 ep1_cnt;
    volatile u32 ep2_cnt;
    volatile u32 ep3_cnt;
    volatile u32 ep4_cnt;
    volatile u32 ep5_cnt;
    volatile u32 ep6_cnt;
    volatile u32 ep0_adr ;
    volatile u32 ep1_tadr;
    volatile u32 ep1_radr;
    volatile u32 ep2_tadr;
    volatile u32 ep2_radr;
    volatile u32 ep3_tadr;
    volatile u32 ep3_radr;
    volatile u32 ep4_tadr;
    volatile u32 ep4_radr;
    volatile u32 ep5_tadr;
    volatile u32 ep5_radr;
    volatile u32 ep6_tadr;
    volatile u32 ep6_radr;
    volatile u32 com_con;
    volatile u32 reserved0;
    volatile u32 phy_con0;
    volatile u32 phy_con1;
    volatile u32 phy_con2;
    volatile u32 iso_con0;
    volatile u32 iso_con1;
};

struct ep_regs {
    volatile u16  txmaxp    ;
    volatile u16  txcsr     ;
    volatile u16  rxmaxp    ;
    volatile u16  rxcsr     ;
    volatile u16  rxcount   ;
    volatile u8   txtype    ;
    volatile u8   txinterval;
    volatile u8   rxtype    ;
    volatile u8   rxinterval;
    volatile u8   reserve;
    volatile u8   fifosize  ;
};

struct hub_regs {
    volatile u8 txfuncaddr;
    volatile u8 unused_0;
    volatile u8 txhubaddr;
    volatile u8 txhubport;
    volatile u8 rxfuncaddr;
    volatile u8 unused_1;
    volatile u8 rxhubaddr;
    volatile u8 rxhubport;
};


struct musb_hw_ep {
    struct sie_regs *sie;
    struct ep_regs *ep[USB_EP_MAX];
    struct musb_regs *regs;
    struct hub_regs *hregs[USB_EP_MAX];
    volatile u8 *const devctl;
    volatile u8 *const fifo0;
    volatile u16 *tx_dpktdis;
};

#if 0

volatile u16 *const csr0;
volatile u16 *const count0;
volatile u8 *const type0;
#endif


#define USB_EP(ep)  ((struct ep_regs*)ep##TXMAXP)

#define USB_SIE(x) ((struct sie_regs*)&x##_CON)

#define USB_REGS(x) ((struct musb_regs*)x##_FADDR)

#define USB_HUB_REGS(x) ((struct hub_regs*)x##_TXFUNCADDR)

#define MULTIPOINT	1

#define DEFINE_USB_PHY(phy)\
const struct musb_hw_ep phy##_phy_desc=\
	{\
		.sie = USB_SIE(phy##_SIE),\
		.ep[0] =USB_EP(phy##_EP0),\
		.ep[1] =USB_EP(phy##_EP1),\
		.ep[2] =USB_EP(phy##_EP2),\
		.ep[3] =USB_EP(phy##_EP3),\
		.ep[4] =USB_EP(phy##_EP4),\
		.ep[5] =USB_EP(phy##_EP5),\
		.ep[6] =USB_EP(phy##_EP6),\
		.regs = USB_REGS(phy),\
		.hregs[0] = USB_HUB_REGS(phy##_EP0),\
		.hregs[1] = USB_HUB_REGS(phy##_EP1),\
		.hregs[2] = USB_HUB_REGS(phy##_EP2),\
		.hregs[3] = USB_HUB_REGS(phy##_EP3),\
		.hregs[4] = USB_HUB_REGS(phy##_EP4),\
		.hregs[5] = USB_HUB_REGS(phy##_EP5),\
		.hregs[6] = USB_HUB_REGS(phy##_EP6),\
		.devctl = phy##_DEVCTL,\
		.fifo0 = phy##_FIFO0,\
        .tx_dpktdis = phy##_TX_DPKTDIS,\
	};
#if 0
.type0 = phy##_TYPE0,\
         .csr0 = phy##_CSR0,\
                 .count0 = phy##_COUNT0,\

#endif

#define USB_MSD_SCSI_DATA \
    { \
        0x00, \
        0x80, \
        0x02, \
        0x02, \
        0x1F, \
        0x00, 0x00, 0x00, \
        'D',  \
        'V',  \
        '1',  \
        '5',  \
        ' ',  \
        ' ',  \
        ' ',  \
        ' ',  \
        ' ',  \
        'D',  \
        'E',  \
        'V',  \
        'I',  \
        'C',  \
        'E',  \
        ' ',  \
        'V',  \
        '1',  \
        '.',  \
        '0',  \
        '0',  \
        ' ',  \
        ' ',  \
        ' ',  \
        0x31, \
        0x2e, \
        0x30, \
        0x30  \
    }

#define USB_UVC_SLAVE_MULTI_CNT 8  //UVC多包发送 开，注释该宏关闭特性

#endif
