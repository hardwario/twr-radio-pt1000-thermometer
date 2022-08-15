#ifndef _TWR_ADS122C04_H
#define _TWR_ADS122C04_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

#define TWR_ADS122C04_ADDRESS_A     0x40
#define TWR_ADS122C04_ADDRESS_B     0x41

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
