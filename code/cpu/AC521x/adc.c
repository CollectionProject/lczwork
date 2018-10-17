#include "asm/cpu.h"
#include "asm/adc.h"
#include "system/init.h"
#include "system/timer.h"

#define FIRST_CHANNEL() \
	adc_scan_begin

#define NEXT_CHANNEL(ch) \
	++ch >= adc_scan_end? adc_scan_begin:ch

static struct adc_scan *channel = FIRST_CHANNEL();

extern void adc_spin_lck();
extern void adc_spin_unlck();
static void adc_scan_process(void *p)
{
#if 0
    adc_spin_lck();
    channel = NEXT_CHANNEL(channel);
    ADCSEL(channel->channel);
    KITSTART();
    while (!ADC_PND());
    channel->value = GPADC_RES;
    /* printf(">>>>>>>>>>>>>>>>> ch: %d val:0x%x\n",channel->channel,channel->value); */
    adc_spin_unlck();
#else
    while (!ADC_PND());
    channel->value = GPADC_RES;
    if (channel->updata) {
        channel->updata();
    }

    channel = NEXT_CHANNEL(channel);
    /* printf(">>>>>>>>>>>>>>>>> ch: %d val:0x%x\n",channel->channel,channel->value); */
    ADCSEL(channel->channel);
    KITSTART();
#endif
}


static int adc_scan_init()
{
    ADCSEL(channel->channel);
    ADC_EN(1);
    ADCEN(1);
    ADC_BAUD(0x7);
    ADC_WTIME(0x1);
    KITSTART();

    //LDO_CON |= BIT(11);

    sys_hi_timer_add(0, adc_scan_process, 4);

    return 0;
}
platform_initcall(adc_scan_init);




