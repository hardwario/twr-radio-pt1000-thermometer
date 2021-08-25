#include <application.h>

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000) // 7 hodin
#define PT1000_UPDATE_INTERVAL (10 * 60 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * 60 * 1000)

//#define PT1000_UPDATE_INTERVAL (5000)

// LED instance
twr_led_t led;

twr_chester_x3_t x3;

twr_tick_t next_pub;

twr_button_t button;

twr_scheduler_task_id_t send_temperature;

void x3_event_handler(twr_chester_x3_t *self, twr_chester_x3_event_t event, void *event_param)
{
    if (event == TWR_CHESTER_X3_EVENT_UPDATE)
    {
        float temperature;
        twr_chester_x3_get_temperature_1(self, &temperature);
        twr_log_debug("Temperature: %.2f", temperature);

        if (next_pub < twr_scheduler_get_spin_tick())
        {
            twr_log_debug("Temperature: %.2f", temperature);
            twr_radio_pub_float("temperature", &temperature);
            next_pub = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float voltage;
    int percentage;

    if (event == TWR_MODULE_BATTERY_EVENT_UPDATE)
    {
        if (twr_module_battery_get_voltage(&voltage))
        {
            twr_radio_pub_battery(&voltage);
        }

        /*if (twr_module_battery_get_charge_level(&percentage))
        {
            values.battery_pct = percentage;
        }*/
    }
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_chester_x3_measure(&x3);
    }
}

// Application initialization function which is called once after boot
void application_init(void)
{
    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, 0);
    twr_led_pulse(&led, 2000);

    twr_chester_x3_init(&x3, TWR_I2C_I2C0, twr_chester_x3_I2C_ADDRESS);
    twr_chester_x3_set_event_handler(&x3, x3_event_handler, NULL);
    twr_chester_x3_set_update_interval(&x3, PT1000_UPDATE_INTERVAL);

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, 0);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    // Initialize radio
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("pt1000-thermometer", VERSION);
}
