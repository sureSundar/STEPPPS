// WINDSURF: IP stack stub
#include <stdint.h>

int ip_init(void) { return 0; }
int ip_send(uint32_t dst, const void* payload, uint16_t len) { (void)dst; (void)payload; (void)len; return 0; }
int ip_recv(void* payload, uint16_t maxlen) { (void)payload; (void)maxlen; return 0; }
