#ifndef __SPI_H__
#define __SPI_H__


#include "typedef.h"


/*enum spi_mode {
    SPI_2WIRE_MODE,
    SPI_ODD_MODE,
    SPI_DUAL_MODE,
    SPI_QUAD_MODE,
};*/

#define IOCTL_SPI_SET_CS            _IOW('S', 1, 1)
#define IOCTL_SPI_SEND_BYTE         _IOW('S', 2, 1)
#define IOCTL_SPI_SEND_CMD          _IOW('S', 3, 1)
#define IOCTL_SPI_GET_BIT_MODE      _IOW('S', 4, 1)
#define IOCTL_SPI_READ_BYTE         _IOW('S', 5, 1)
#define IOCTL_SPI_SET_CRC           _IOW('S', 6, 1)
#define IOCTL_SPI_READ_CRC          _IOR('S', 7, 1)



struct spi_io {
    u8 cs_pin;
    u8 di_pin;
    u8 do_pin;
    u8 clk_pin;
    u8 d2_pin;
    u8 d3_pin;
};

struct spi_regs {
    volatile u32 con;
    volatile u8  baud;
    volatile u8  reg1[3];
    volatile u8  buf;
    volatile u8  reg2[3];
    volatile u32 adr;
    volatile u32 cnt;
};

struct spi_platform_data {
    u8 port;
    u8 mode;
    u8 irq;
    u32 clk;
    const struct spi_io *io;
    volatile struct spi_regs *reg;
    void (*init)(const struct spi_platform_data *);
};

#define SPI0_PLATFORM_DATA_BEGIN(spi0_data) \
    static const struct spi_io spi0_io[] = { \
        { \
            .cs_pin     = IO_PORTA_00, \
            .di_pin     = IO_PORTA_01, \
            .do_pin     = IO_PORTA_03, \
            .clk_pin    = IO_PORTA_04, \
            .d2_pin     = IO_PORTA_02, \
            .d3_pin     = IO_PORTH_13, \
        }, \
        { \
            .cs_pin     = IO_PORTH_06, \
            .di_pin     = IO_PORTH_07, \
            .do_pin     = IO_PORTH_09, \
            .clk_pin    = IO_PORTH_10, \
            .d2_pin     = IO_PORTH_08, \
            .d3_pin     = IO_PORTH_11, \
        }, \
    }; \
    static void __spi0_iomc_init(const struct spi_platform_data *pd) \
    { \
        IOMC1 &= ~BIT(1); \
        if (pd->port == 'A') { \
           IOMC0 &= ~BIT(15); \
        } else { \
           IOMC0 |= BIT(15); \
        } \
    }\
    static const struct spi_platform_data spi0_data = { \
        .irq = SPI0_INT, \



#define SPI0_PLATFORM_DATA_END() \
    .io     = spi0_io, \
    .reg    = (volatile struct spi_regs *)&SPI0_CON, \
    .init   = __spi0_iomc_init, \
};





#define SPI1_PLATFORM_DATA_BEGIN(spi1_data) \
    static const struct spi_io spi1_io[] = { \
        { \
            .cs_pin     = -1, \
            .di_pin     = IO_PORTH_05, \
            .do_pin     = IO_PORTH_04, \
            .clk_pin    = IO_PORTH_03, \
            .d2_pin     = -1, \
            .d3_pin     = -1, \
        }, \
        { \
            .cs_pin     = -1, \
            .di_pin     = IO_PORTD_02, \
            .do_pin     = IO_PORTD_01, \
            .clk_pin    = IO_PORTD_00, \
            .d2_pin     = -1, \
            .d3_pin     = -1, \
        }, \
        { \
            .cs_pin     = -1, \
            .di_pin     = IO_PORTG_05, \
            .do_pin     = IO_PORTG_07, \
            .clk_pin    = IO_PORTG_06, \
            .d2_pin     = -1, \
            .d3_pin     = -1, \
        }, \
        { \
            .cs_pin     = -1, \
            .di_pin     = IO_PORTF_04, \
            .do_pin     = IO_PORTF_06, \
            .clk_pin    = IO_PORTF_05, \
            .d2_pin     = -1, \
            .d3_pin     = -1, \
        }, \
    }; \
    static void __spi1_iomc_init(const struct spi_platform_data *pd) \
    { \
        IOMC1 &= ~(BIT(4) | BIT(5)); \
        if (pd->port == 'B') { \
           IOMC1 |= BIT(4); \
        } else if (pd->port == 'C') { \
           IOMC1 |= BIT(5); \
        } else if (pd->port == 'D') { \
           IOMC1 |= BIT(4) | BIT(5); \
        } \
    }\
    static const struct spi_platform_data spi1_data = { \
        .irq = SPI1_INT, \



#define SPI1_PLATFORM_DATA_END() \
    .io     = spi1_io, \
    .reg    = (volatile struct spi_regs *)&SPI1_CON, \
    .init   = __spi1_iomc_init, \
};



#if 0
#define SPI2_PLATFORM_DATA_BEGIN(spi2_data) \
    static const struct spi_io spi2_io[] = { \
        { \
            .cs_pin     = IO_PORTD_05, \
            .di_pin     = IO_PORTD_02, \
            .do_pin     = IO_PORTD_01, \
            .clk_pin    = IO_PORTD_00, \
            .d2_pin     = IO_PORTD_03, \
            .d3_pin     = IO_PORTD_04, \
        }, \
        { \
            .cs_pin     = IO_PORTB_05, \
            .di_pin     = IO_PORTB_06, \
            .do_pin     = IO_PORTB_08, \
            .clk_pin    = IO_PORTB_09, \
            .d2_pin     = IO_PORTB_07, \
            .d3_pin     = IO_PORTB_10, \
        }, \
    }; \
    static void __spi2_iomc_init(const struct spi_platform_data *pd) \
    { \
        if (pd->port == SPI_PORTD_0_5) { \
           IOMC2 &= ~(BIT(28) | BIT(29)); \
        } else { \
           IOMC2 |= BIT(28); \
           IOMC2 &= ~BIT(29); \
        } \
    }\
    static const struct spi_platform_data spi2_data = { \
        .irq = SPI2_INT, \



#define SPI2_PLATFORM_DATA_END() \
    .io     = spi2_io, \
    .reg    = (volatile struct spi_regs *)&SPI2_CON, \
    .init   = __spi2_iomc_init, \
};
#endif


extern const struct device_operations spi_dev_ops;


#endif

