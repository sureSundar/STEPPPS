// WINDSURF: TCP stub
#include <stdint.h>

int tcp_init(void) { return 0; }
int tcp_connect(uint32_t addr, uint16_t port) { (void)addr; (void)port; return 0; }
int tcp_send(const void* data, uint16_t len) { (void)data; (void)len; return 0; }
int tcp_recv(void* data, uint16_t maxlen) { (void)data; (void)maxlen; return 0; }
