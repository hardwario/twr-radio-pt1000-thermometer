#ifndef _TWR_ADS122C04_H
#define _TWR_ADS122C04_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

#define ADS122C04_RESET         0x06
#define ADS122C04_START_SYNC    0x08
#define ADS122C04_POWERDOWN     0x02
#define ADS122C04_READ_DATA     0x10
#define ADS122C04_READ_REG      0x20
#define ADS122C04_WRITE_REG     0x40

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

typedef struct
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _address;

} twr_ads122c04_t;

bool twr_ads122c04_init(twr_ads122c04_t *ctx, twr_i2c_channel_t i2c_channel, uint8_t address);
bool twr_ads122c04_reset(twr_ads122c04_t *ctx);
bool twr_ads122c04_start_sync(twr_ads122c04_t *ctx);
bool twr_ads122c04_powerdown(twr_ads122c04_t *ctx);
bool twr_ads122c04_register_read(twr_ads122c04_t *ctx, uint8_t address, uint8_t *data);
bool twr_ads122c04_register_write(twr_ads122c04_t *ctx, uint8_t address, uint8_t data);
bool twr_ads122c04_data_read(twr_ads122c04_t *ctx, uint32_t *data);
bool twr_ads122c04_data_read_int32(twr_ads122c04_t *ctx, int32_t *data);
bool twr_ads122c04_measure(twr_ads122c04_t *ctx);
bool twr_ads122c04_read(twr_ads122c04_t *ctx, float *temperature);

#endif
