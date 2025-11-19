#ifndef TBOS_SENSOR_BUS_H
#define TBOS_SENSOR_BUS_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    SENSOR_IF_WIFI,
    SENSOR_IF_BLUETOOTH,
    SENSOR_IF_SERIAL,
    SENSOR_IF_COM1,
    SENSOR_IF_P2P,
    SENSOR_IF_HTTP,
    SENSOR_IF_HTTPS,
    SENSOR_IF_SSL,
    SENSOR_IF_UDP,
    SENSOR_IF_TCP,
} sensor_interface_t;

typedef struct {
    uint64_t timestamp_us;
    sensor_interface_t interface;
    char source[64];
    char destination[64];
    char metadata[128];
    uint8_t payload[1024];
    size_t payload_len;
} sensor_envelope_t;

typedef void (*sensor_bus_callback_t)(const sensor_envelope_t* envelope, void* user_data);

int sensor_bus_init(void);
int sensor_bus_subscribe(sensor_interface_t interface, sensor_bus_callback_t cb, void* user_data);
int sensor_bus_publish(const sensor_envelope_t* envelope);

#endif /* TBOS_SENSOR_BUS_H */
