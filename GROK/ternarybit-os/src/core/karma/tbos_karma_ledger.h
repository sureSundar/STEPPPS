#ifndef TBOS_KARMA_LEDGER_H
#define TBOS_KARMA_LEDGER_H

#include "../tbos_shutdown.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void tbos_karma_ledger_init(void);
int tbos_karma_ledger_get_total(const char* persona, karma_score_t* out_total);
void tbos_karma_ledger_record(const char* persona,
                              karma_score_t delta,
                              karma_score_t new_total,
                              const char* reason);
void tbos_karma_ledger_print(const char* persona, size_t max_entries);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_KARMA_LEDGER_H */
