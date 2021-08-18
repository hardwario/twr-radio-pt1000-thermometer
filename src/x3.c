#include <x3.h>
#include <twr_log.h>

#define _TWR_X3_DELAY_RUN 100

static void _twr_x3_task_interval(void *param);
static void _twr_x3_task_measure(void *param);

void twr_x3_init(twr_x3_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));
    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = twr_scheduler_register(_twr_x3_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_x3_task_measure, self, _TWR_X3_DELAY_RUN);

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);
}

void twr_x3_set_event_handler(twr_x3_t *self, void (*event_handler)(twr_x3_t *, twr_x3_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_x3_set_update_interval(twr_x3_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_x3_measure(self);
    }
}

static void _twr_x3_task_interval(void *param)
{
    twr_x3_t *self = param;

    twr_x3_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

void twr_x3_get_temperature1(twr_x3_t *self, float *temperature)
{
    *temperature = self->temperature1;
}

void twr_x3_get_temperature2(twr_x3_t *self, float *temperature)
{
    *temperature = self->temperature2;
}

bool twr_x3_measure(twr_x3_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

static void _twr_x3_task_measure(void *param)
{
    twr_x3_t *self = param;
start:

    switch (self->_state)
    {
        case TWR_X3_STATE_ERROR:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_X3_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_X3_STATE_INITIALIZE;

            return;
        }
        case TWR_X3_STATE_INITIALIZE:
        {
            self->_state = TWR_X3_STATE_ERROR;

            if (!twr_ads122c04_init(&self->ads122c04_a, self->_i2c_channel, 0x40))
            {
                goto start;
            }
            /*if(!twr_ads122c04_init(&self->ads122c04_b), self->_i2c_channel, 0x41))
            {
                goto start;
            }*/

            self->_state = TWR_X3_STATE_MEASURE;

            self->_tick_ready = twr_tick_get() + 50;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_X3_STATE_MEASURE:
        {
            self->_state = TWR_X3_STATE_ERROR;
            //bool error2 = !twr_ads122c04_measure(&(self->ads122c04_b), &temperature2);

            /*self->temperature1 = temperature1;
            self->temperature2 = temperature2;*/

            //self->_state = TWR_X3_STATE_UPDATE;

            //twr_log_debug("TIMEOUT: %d", self->ads122c04_a._measurement_timeout);

            if (!twr_ads122c04_measure(&self->ads122c04_a))
            {
                goto start;
            }

            self->_state = TWR_X3_STATE_READ;

            self->_tick_ready = twr_tick_get() + 120;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_X3_STATE_READ:
        {
            self->_state = TWR_X3_STATE_ERROR;

            float temperature1;
            if (!twr_ads122c04_read(&self->ads122c04_a, &temperature1))
            {
                goto start;
            }

            self->temperature1 = temperature1;

            self->_state = TWR_X3_STATE_UPDATE;
            twr_scheduler_plan_current_now();

            return;
        }
        case TWR_X3_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_X3_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_X3_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = TWR_X3_STATE_ERROR;

            goto start;
        }
    }
}
