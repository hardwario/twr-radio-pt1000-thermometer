#ifndef _TWR_X3_H
#define _TWR_X3_H

#include <twr_i2c.h>
#include <twr_scheduler.h>
#include <ads122c04.h>


#define TWR_X3_I2C_ADDRESS 0x48

typedef enum
{
    //! @brief Error event
    TWR_X3_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_X3_EVENT_UPDATE = 1

} twr_x3_event_t;

typedef enum
{
    TWR_X3_STATE_ERROR = -1,
    TWR_X3_STATE_INITIALIZE = 0,
    TWR_X3_STATE_MEASURE = 1,
    TWR_X3_STATE_READ = 2,
    TWR_X3_STATE_UPDATE = 3

} twr_x3_state_t;

typedef struct twr_x3_t twr_x3_t;

struct twr_x3_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_tick_t _update_interval;
    twr_x3_state_t _state;
    void (*_event_handler)(twr_x3_t *, twr_x3_event_t, void *);
    void *_event_param;
    twr_tick_t _tick_ready;
    bool _measurement_active;
    twr_ads122c04_t ads122c04_a;
    twr_ads122c04_t ads122c04_b;
    bool ads122c04_a_is_present;
    bool ads122c04_b_is_present;
    float ads122c04_a_temperature;
    float ads122c04_b_temperature;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
};

void twr_x3_init(twr_x3_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);
void twr_x3_set_update_interval(twr_x3_t *self, twr_tick_t interval);
void twr_x3_set_event_handler(twr_x3_t *self, void (*event_handler)(twr_x3_t *, twr_x3_event_t, void *), void *event_param);
bool twr_x3_measure(twr_x3_t *self);
void twr_x3_get_temperature1(twr_x3_t *self, float *temperature);
void twr_x3_get_temperature2(twr_x3_t *self, float *temperature);

#endif
