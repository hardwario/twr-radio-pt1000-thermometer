#include <twr_ads122c04.h>

#define ADS122C04_RESET         0x06
#define ADS122C04_START_SYNC    0x08
#define ADS122C04_POWERDOWN     0x02
#define ADS122C04_READ_DATA     0x10
#define ADS122C04_READ_REG      0x20
#define ADS122C04_WRITE_REG     0x40

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

static double temperature_sensor_resistance_ohm_to_temperature_celsius(double r, double r0, double a, double b);

bool twr_ads122c04_init(twr_ads122c04_t *ctx, twr_i2c_channel_t i2c_channel, uint8_t address)
{
    ctx->_i2c_channel = i2c_channel;
    ctx->_address = address;

    return twr_ads122c04_powerdown(ctx);
}

bool twr_ads122c04_reset(twr_ads122c04_t *ctx)
{
    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = ctx->_address;
    transfer.memory_address = ADS122C04_RESET;
    transfer.length = 0;

    return twr_i2c_memory_write(ctx->_i2c_channel, &transfer);
}

bool twr_ads122c04_start_sync(twr_ads122c04_t *ctx)
{
    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = ctx->_address;
    transfer.memory_address = ADS122C04_START_SYNC;
    transfer.length = 0;

    return twr_i2c_memory_write(ctx->_i2c_channel, &transfer);
}

bool twr_ads122c04_powerdown(twr_ads122c04_t *ctx)
{
    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = ctx->_address;
    transfer.memory_address = ADS122C04_POWERDOWN;
    transfer.length = 0;

    return twr_i2c_memory_write(ctx->_i2c_channel, &transfer);
}

bool twr_ads122c04_register_read(twr_ads122c04_t *ctx, uint8_t address, uint8_t *data)
{
    return twr_i2c_memory_read_8b(ctx->_i2c_channel, ctx->_address, ADS122C04_READ_REG | (address << 2), data);
}

bool twr_ads122c04_register_write(twr_ads122c04_t *ctx, uint8_t address, uint8_t data)
{
    return twr_i2c_memory_write_8b(ctx->_i2c_channel, ctx->_address, ADS122C04_WRITE_REG | (address << 2), data);
}

bool twr_ads122c04_data_read(twr_ads122c04_t *ctx, uint32_t *data)
{
    uint8_t buffer[3];

    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = ctx->_address;
    transfer.memory_address = ADS122C04_READ_DATA;
    transfer.buffer = buffer;
    transfer.length = 3;

    if (!twr_i2c_memory_read(ctx->_i2c_channel, &transfer))
    {
        return false;
    }

    *data = buffer[0] << 16 | buffer[1] << 8 | buffer[2];

    return true;
}

bool twr_ads122c04_data_read_int32(twr_ads122c04_t *ctx, int32_t *data)
{
    uint8_t buffer[3];

    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = ctx->_address;
    transfer.memory_address = ADS122C04_READ_DATA;
    transfer.buffer = buffer;
    transfer.length = 3;

    if (!twr_i2c_memory_read(ctx->_i2c_channel, &transfer))
    {
        return false;
    }

    *data = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8;
    *data >>= 8;

    return true;
}

static double temperature_sensor_resistance_ohm_to_temperature_celsius(double r, double r0, double a, double b)
{
    return (sqrt((a * a * r0) - (4 * b * r0) + (4 * b * r)) - (a * sqrt(r0))) / (2 * b * sqrt(r0));
}

bool twr_ads122c04_measure(twr_ads122c04_t *ctx)
{
    if (!twr_ads122c04_reset(ctx))
    {
        return false;
    }

    // Set AINp = AIN1, AINn = AIN0
    twr_ads122c04_register_write(ctx, 0x00, 0x30);
    // Set external reference REFP, REFN
    twr_ads122c04_register_write(ctx, 0x01, 0x02);
    // Set IDAC 1000uA
    twr_ads122c04_register_write(ctx, 0x02, 0x06);
    // Set IDAC1 to AIN3
    twr_ads122c04_register_write(ctx, 0x03, 0x80);

    twr_ads122c04_start_sync(ctx);

    return true;
}

bool twr_ads122c04_read(twr_ads122c04_t *ctx, float *temperature)
{
    uint8_t cr2;
    uint32_t data;

    twr_ads122c04_register_read(ctx, 0x02, &cr2);

    twr_ads122c04_data_read(ctx, &data);

    float r_ref = 1500.0f;
    float gain = 1.0f;
    float r = r_ref * (data / (gain * (1 << 23)));

    *temperature = temperature_sensor_resistance_ohm_to_temperature_celsius(r, 1000, RTD_A, RTD_B);

    return twr_ads122c04_powerdown(ctx);
}
