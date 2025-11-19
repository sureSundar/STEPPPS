#include "tbos_supershell.h"
#include "tbos_shell.h"
#include <stdio.h>

#if defined(TBOS_PACKAGE_NETWORK_CORE)
static int supershell_network_entry(const char* persona) {
    printf("[Supershell] Loading Sangha persona '%s' with conscious networking tools...\n",
           persona ? persona : "network_sangha");
    return tbos_shell_run();
}
#endif

#if defined(TBOS_PACKAGE_FILESYSTEM_UNIVERSAL)
static int supershell_fs_entry(const char* persona) {
    printf("[Supershell] Loading universal filesystem persona '%s' with UCFS/PXFS overlays...\n",
           persona ? persona : "fs_universal");
    return tbos_shell_run();
}
#endif

void tbos_supershell_register_packages(void) {
#if defined(TBOS_PACKAGE_NETWORK_CORE)
    static const tbos_supershell_mode_t network_mode = {
        .persona_id = "network_sangha",
        .description = "Sangha-aware shell with conscious networking commands",
        .entry = supershell_network_entry,
    };
    tbos_supershell_register(&network_mode);
#endif

#if defined(TBOS_PACKAGE_FILESYSTEM_UNIVERSAL)
    static const tbos_supershell_mode_t fs_mode = {
        .persona_id = "fs_universal",
        .description = "Universal filesystem shell with UCFS/PXFS utilities",
        .entry = supershell_fs_entry,
    };
    tbos_supershell_register(&fs_mode);
#endif

    /* Additional package-driven personas can be registered above */
}
