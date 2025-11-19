#include "sensor_bus.h"
#include <string.h>

#define MAX_SUBSCRIBERS 16

typedef struct {
    sensor_interface_t interface;
    sensor_bus_callback_t callback;
    void* user_data;
} sensor_subscription_t;

static sensor_subscription_t subscriptions[MAX_SUBSCRIBERS];
static size_t subscription_count = 0;

int sensor_bus_init(void) {
    subscription_count = 0;
    memset(subscriptions, 0, sizeof(subscriptions));
    return 0;
}

int sensor_bus_subscribe(sensor_interface_t interface, sensor_bus_callback_t cb, void* user_data) {
    if (!cb || subscription_count >= MAX_SUBSCRIBERS) {
        return -1;
    }
    subscriptions[subscription_count].interface = interface;
    subscriptions[subscription_count].callback = cb;
    subscriptions[subscription_count].user_data = user_data;
    subscription_count++;
    return 0;
}

int sensor_bus_publish(const sensor_envelope_t* envelope) {
    if (!envelope) return -1;
    for (size_t i = 0; i < subscription_count; i++) {
        if (subscriptions[i].interface == envelope->interface) {
            subscriptions[i].callback(envelope, subscriptions[i].user_data);
        }
    }
    return 0;
}
