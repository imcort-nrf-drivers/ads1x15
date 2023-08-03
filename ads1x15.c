#include "ads1x15.h"

//  REGISTERS
#define ADS1X15_REG_CONVERT         0x00
#define ADS1X15_REG_CONFIG          0x01
#define ADS1X15_REG_LOW_THRESHOLD   0x02
#define ADS1X15_REG_HIGH_THRESHOLD  0x03

//  CONFIG REGISTER

//  BIT 15      Operational Status           // 1 << 15
#define ADS1X15_OS_BUSY             0x0000
#define ADS1X15_OS_NOT_BUSY         0x8000
#define ADS1X15_OS_START_SINGLE     0x8000

//  BIT 12-14   read differential
#define ADS1X15_CHANNEL_MASK        0x3800

#define ADS1X15_MUX_DIFF_0_1        0x0000
#define ADS1X15_MUX_DIFF_0_3        0x1000
#define ADS1X15_MUX_DIFF_1_3        0x2000
#define ADS1X15_MUX_DIFF_2_3        0x3000
//              read single
#define ADS1X15_READ_0              0x4000   //  pin << 12
#define ADS1X15_READ_1              0x5000   //  pin = 0..3
#define ADS1X15_READ_2              0x6000
#define ADS1X15_READ_3              0x7000


//  BIT 9-11    gain                         //  (0..5) << 9
#define ADS1X15_PGA_6_144V          0x0000   //  voltage
#define ADS1X15_PGA_4_096V          0x0200   //
#define ADS1X15_PGA_2_048V          0x0400   //  default
#define ADS1X15_PGA_1_024V          0x0600
#define ADS1X15_PGA_0_512V          0x0800
#define ADS1X15_PGA_0_256V          0x0A00

//  BIT 8       mode                         //  1 << 8
#define ADS1X15_MODE_CONTINUE       0x0000
#define ADS1X15_MODE_SINGLE         0x0100

//  BIT 5-7     data rate sample per second  // (0..7) << 5
/*
differs for different devices, check datasheet or readme.md

|  data rate  |  ADS101x  |  ADS111x  |   Notes   |
|:-----------:|----------:|----------:|:---------:|
|     0       |   128     |    8      |  slowest  |
|     1       |   250     |    16     |           |
|     2       |   490     |    32     |           |
|     3       |   920     |    64     |           |
|     4       |   1600    |    128    |  default  |
|     5       |   2400    |    250    |           |
|     6       |   3300    |    475    |           |
|     7       |   3300    |    860    |  fastest  |
*/

//  BIT 4 comparator modi                    // 1 << 4
#define ADS1X15_COMP_MODE_TRADITIONAL   0x0000
#define ADS1X15_COMP_MODE_WINDOW        0x0010

//  BIT 3 ALERT active value                 // 1 << 3
#define ADS1X15_COMP_POL_ACTIV_LOW      0x0000
#define ADS1X15_COMP_POL_ACTIV_HIGH     0x0008

//  BIT 2 ALERT latching                     // 1 << 2
#define ADS1X15_COMP_NON_LATCH          0x0000
#define ADS1X15_COMP_LATCH              0x0004

//  BIT 0-1 ALERT mode                       // (0..3)
#define ADS1X15_COMP_QUE_1_CONV         0x0000  //  trigger alert after 1 convert
#define ADS1X15_COMP_QUE_2_CONV         0x0001  //  trigger alert after 2 converts
#define ADS1X15_COMP_QUE_4_CONV         0x0002  //  trigger alert after 4 converts
#define ADS1X15_COMP_QUE_NONE           0x0003  //  disable comparator

uint8_t ads1x15_addr[4] = {ADS1X15_ADDRESS_1, ADS1X15_ADDRESS_2, ADS1X15_ADDRESS_3, ADS1X15_ADDRESS_4};
uint16_t ads1x15_config[4] = {0,0,0,0};

void ads1x15_writeRegister(uint8_t address, uint8_t reg, uint16_t value)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (value >> 8);
    buf[2] = (value & 0xFF);
    
    NRF_LOG_INFO("addr %x",address);
    NRF_LOG_HEXDUMP_INFO(buf,3);
  
    iic_send(address, buf, 3, false);
}


uint16_t ads1x15_readRegister(uint8_t address, uint8_t reg)
{
    uint8_t buf[2];
    buf[0] = reg;
  
    iic_send(address, buf, 1, false);
    iic_read(address, buf, 2);

    uint16_t value = buf[0] << 8;
    value += buf[1];
    return value;
    
}

void ads1x15_config_reg(uint8_t device, uint16_t config)
{
    ads1x15_config[device] = config;
    
    ads1x15_writeRegister(ads1x15_addr[device], ADS1X15_REG_CONFIG, ads1x15_config[device]);
    
}

void ads1x15_startConv(uint8_t device)
{
    ads1x15_config[device] |= ADS1X15_OS_START_SINGLE;
    ads1x15_writeRegister(ads1x15_addr[device], ADS1X15_REG_CONFIG, ads1x15_config[device]);
    ads1x15_config[device] &= (~ADS1X15_OS_START_SINGLE);
}

uint16_t ads1x15_readResult(uint8_t device)
{
    return ads1x15_readRegister(ads1x15_addr[device], ADS1X15_REG_CONVERT);
}

void ads1x15_setChannel(uint8_t device, uint8_t chn)
{
    uint16_t chn_reg;
    switch(chn)
    {
        case 0:
            chn_reg = ADS1X15_READ_0;
            break;
        case 1:
            chn_reg = ADS1X15_READ_1;
            break;
        case 2:
            chn_reg = ADS1X15_READ_2;
            break;
        case 3:
            chn_reg = ADS1X15_READ_3;
            break;
    
    }
    ads1x15_config[device] &= (~ADS1X15_CHANNEL_MASK);
    ads1x15_config[device] |= chn_reg;
    ads1x15_writeRegister(ads1x15_addr[device], ADS1X15_REG_CONFIG, ads1x15_config[device]);

}

void ads1x15_begin(void)
{
    iic_init();
    ads1x15_config_reg(0, ADS1X15_PGA_2_048V | ADS1X15_MODE_SINGLE | (0 << 5) | ADS1X15_COMP_MODE_TRADITIONAL | ADS1X15_COMP_POL_ACTIV_LOW | ADS1X15_COMP_NON_LATCH | ADS1X15_COMP_QUE_NONE);
    //ads1x15_config_reg(1, ADS1X15_PGA_2_048V | ADS1X15_MODE_SINGLE | (0 << 5) | ADS1X15_COMP_MODE_TRADITIONAL | ADS1X15_COMP_POL_ACTIV_LOW | ADS1X15_COMP_NON_LATCH | ADS1X15_COMP_QUE_NONE);
    
}

