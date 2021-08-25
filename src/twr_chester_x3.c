#include <twr_chester_x3.h>

#define _twr_chester_x3_DELAY_RUN 100

static void _twr_chester_x3_task_interval(void *param);
static void _twr_chester_x3_task_measure(void *param);

void twr_chester_x3_init(twr_chester_x3_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));
    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = twr_scheduler_register(_twr_chester_x3_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_chester_x3_task_measure, self, _twr_chester_x3_DELAY_RUN);

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);
}

void twr_chester_x3_set_event_handler(twr_chester_x3_t *self, void (*event_handler)(twr_chester_x3_t *, twr_chester_x3_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_chester_x3_set_update_interval(twr_chester_x3_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_chester_x3_measure(self);
    }
}

static void _twr_chester_x3_task_interval(void *param)
{
    twr_chester_x3_t *self = param;

    twr_chester_x3_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

void twr_chester_x3_get_temperature_1(twr_chester_x3_t *self, float *temperature)
{
    if (self->ads122c04_1_is_present)
    {
        *temperature = self->ads122c04_1_temperature;
    }
    else
    {
        temperature = NULL;
    }
}

void twr_chester_x3_get_temperature_2(twr_chester_x3_t *self, float *temperature)
{
    if (self->ads122c04_2_is_present)
    {
        *temperature = self->ads122c04_2_temperature;
    }
    else
    {
        temperature = NULL;
    }
}

bool twr_chester_x3_measure(twr_chester_x3_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

static void _twr_chester_x3_task_measure(void *param)
{
    twr_chester_x3_t *self = param;
start:

    switch (self->_state)
    {
        case TWR_CHESTER_X3_STATE_ERROR:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_CHESTER_X3_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_CHESTER_X3_STATE_INITIALIZE;

            return;
        }
        case TWR_CHESTER_X3_STATE_INITIALIZE:
        {
            self->_state = TWR_CHESTER_X3_STATE_ERROR;

            bool init_state_1 = twr_ads122c04_init(&self->ads122c04_1, self->_i2c_channel, TWR_ADS122C04_ADDRESS_A);
            bool init_state_2 = twr_ads122c04_init(&self->ads122c04_2, self->_i2c_channel, TWR_ADS122C04_ADDRESS_B);

            self->ads122c04_1_is_present = init_state_1;
            self->ads122c04_2_is_present = init_state_2;

            if (!init_state_1 && !init_state_2)
            {
                goto start;
            }

            self->_state = TWR_CHESTER_X3_STATE_MEASURE;

            self->_tick_ready = twr_tick_get() + 50;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_CHESTER_X3_STATE_MEASURE:
        {
            self->_state = TWR_CHESTER_X3_STATE_ERROR;

            bool measure_state_1 = true;
            bool measure_state_2 = true;

            if (self->ads122c04_1_is_present)
            {
                measure_state_1 = twr_ads122c04_measure(&self->ads122c04_1);
            }
            if (self->ads122c04_2_is_present)
            {
                measure_state_2 = twr_ads122c04_measure(&self->ads122c04_2);
            }

            if (!measure_state_1 && !measure_state_2)
            {
                goto start;
            }

            self->_state = TWR_CHESTER_X3_STATE_READ;

            self->_tick_ready = twr_tick_get() + 150;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_CHESTER_X3_STATE_READ:
        {
            self->_state = TWR_CHESTER_X3_STATE_ERROR;

            float temperature_1;
            float temperature_2;

            bool read_state_1 = true;
            bool read_state_2 = false;

            if (self->ads122c04_1_is_present)
            {
                read_state_1 = twr_ads122c04_read(&self->ads122c04_1, &temperature_1);
            }
            if (self->ads122c04_2_is_present)
            {
                read_state_2 = twr_ads122c04_read(&self->ads122c04_2, &temperature_2);
            }

            if (!read_state_1 && !read_state_2)
            {
                goto start;
            }

            if (self->ads122c04_1_is_present && read_state_1)
            {
                self->ads122c04_1_temperature = temperature_1;
            }
            if (self->ads122c04_2_is_present && read_state_2)
            {
                self->ads122c04_2_temperature = temperature_2;
            }

            self->_state = TWR_CHESTER_X3_STATE_UPDATE;
            twr_scheduler_plan_current_now();

            return;
        }
        case TWR_CHESTER_X3_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_CHESTER_X3_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_CHESTER_X3_STATE_INITIALIZE;

            return;
        }
        default:
        {
            self->_state = TWR_CHESTER_X3_STATE_ERROR;

            goto start;
        }
    }
}
