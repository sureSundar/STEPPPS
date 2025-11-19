#ifndef TBOS_SUPERSHELL_H
#define TBOS_SUPERSHELL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*tbos_supershell_entry_t)(const char* persona);

typedef struct {
    const char* persona_id;
    const char* description;
    tbos_supershell_entry_t entry;
} tbos_supershell_mode_t;

int tbos_supershell_init(void);
int tbos_supershell_register(const tbos_supershell_mode_t* mode);
int tbos_supershell_launch(const char* persona_id);
void tbos_supershell_list(void);
const tbos_supershell_mode_t* tbos_supershell_active(void);
void tbos_supershell_register_packages(void);
int tbos_supershell_request_morph(const char* persona_id);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SUPERSHELL_H */
