#ifndef TBOS_LIFECYCLE_H
#define TBOS_LIFECYCLE_H

#include "hal/tbos_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void tbos_lifecycle_genesis(const char* persona_id, const hal_hardware_info_t* info);
void tbos_lifecycle_record_command(const char* cmdline, int result);
bool tbos_lifecycle_should_shutdown(const char** reason_out);
void tbos_lifecycle_session_end(void);
bool tbos_lifecycle_rebirth_pending(const char** persona_out, const char** reason_out);
void tbos_lifecycle_clear_rebirth(void);
void tbos_lifecycle_schedule_rebirth(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_LIFECYCLE_H */
