// WINDSURF: Ethernet driver framework stub
#include <stdint.h>

int eth_init(void) { return 0; }
int eth_send(const void* buf, uint16_t len) { (void)buf; (void)len; return 0; }
int eth_recv(void* buf, uint16_t maxlen) { (void)buf; (void)maxlen; return 0; }
