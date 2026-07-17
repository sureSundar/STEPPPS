#include "steppps/v2/steppps.h"

#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    static const char json[] =
        "{"
        "\"id\":\"scope-test\","
        "\"space\":{"
            "\"coordinates\":{\"lat\":12.5,\"lon\":77.6},"
            "\"device\":\"tbos-test\""
        "},"
        "\"event\":{"
            "\"name\":\"must-not-become-top-level-name\","
            "\"type\":\"unit\","
            "\"description\":\"contains } and \\\"quotes\\\" safely\""
        "},"
        "\"psychology\":{"
            "\"system\":{\"health\":91,\"karma\":108},"
            "\"environment\":{\"weather\":\"clear\",\"temperature\":24.5}"
        "},"
        "\"script\":{\"lang\":\"json\",\"lat\":999}"
        "}";
    steppps_t entity;

    assert(steppps_parse(json, &entity) == 0);
    assert(strcmp(entity.id, "scope-test") == 0);
    assert(entity.name[0] == '\0');
    assert(strcmp(entity.space.device, "tbos-test") == 0);
    assert(fabs(entity.space.lat - 12.5) < 0.000001);
    assert(fabs(entity.space.lon - 77.6) < 0.000001);
    assert(strcmp(entity.event.type, "unit") == 0);
    assert(strcmp(entity.event.description,
                  "contains } and \"quotes\" safely") == 0);
    assert(entity.psych.health == 91);
    assert(entity.psych.karma == 108);
    assert(strcmp(entity.psych.weather, "clear") == 0);
    assert(fabs(entity.psych.temperature - 24.5) < 0.000001);
    assert(strcmp(entity.script.lang, "json") == 0);

    static const char earth_json[] =
        "{"
        "\"kind\":\"human\","
        "\"id\":\"steppps://human/test\","
        "\"name\":\"Test Human\","
        "\"consciousness_level\":\"aware\","
        "\"karma\":108,"
        "\"created\":\"2026-07-16T00:00:00Z\""
        "}";

    assert(steppps_parse(earth_json, &entity) == 0);
    assert(strcmp(entity.kind, "human") == 0);
    assert(strcmp(entity.id, "steppps://human/test") == 0);
    assert(strcmp(entity.time.created, "2026-07-16T00:00:00Z") == 0);
    assert(strcmp(entity.time.utc, "2026-07-16T00:00:00Z") == 0);
    assert(strcmp(entity.psych.consciousness, "aware") == 0);
    assert(entity.psych.karma == 108);

    static const char dimensional_json[] =
        "{"
        "\"steppps_version\":\"1.0\","
        "\"id\":{\"uri\":\"steppps://device/test\"},"
        "\"S_space\":{\"realm\":\"bhulok\",\"device_fingerprint\":\"abc\"},"
        "\"T_time\":{\"created\":\"2026-07-16T01:00:00Z\"},"
        "\"E_event\":{},"
        "\"P_psychology\":{\"consciousness_level\":\"awakening\"},"
        "\"P_pixel\":{\"form\":\"device\"},"
        "\"P_prompt\":{},"
        "\"S_script\":{}"
        "}";

    assert(steppps_parse(dimensional_json, &entity) == 0);
    assert(strcmp(entity.version, "1.0") == 0);
    assert(strcmp(entity.id, "steppps://device/test") == 0);
    assert(strcmp(entity.space.realm, "bhulok") == 0);
    assert(strcmp(entity.space.device, "abc") == 0);
    assert(strcmp(entity.psych.consciousness, "awakening") == 0);
    assert(strcmp(entity.pixel.type, "device") == 0);

    return 0;
}
