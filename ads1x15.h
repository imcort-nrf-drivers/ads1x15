#ifndef __ADS1X15_H__
#define __ADS1X15_H__

#include "transfer_handler.h"

#define ADS1X15_NUM 2

#define ADS1X15_ADDRESS_1 0x48
#define ADS1X15_ADDRESS_2 0x49
#define ADS1X15_ADDRESS_3 0x4A
#define ADS1X15_ADDRESS_4 0x4B

void ads1x15_begin(void);
void ads1x15_setChannel(uint8_t device, uint8_t chn);
void ads1x15_startConv(uint8_t device);
uint16_t ads1x15_readResult(uint8_t device);

#endif
