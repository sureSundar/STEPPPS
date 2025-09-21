// WINDSURF: Socket interface stub
#include <stdint.h>

int sock_open(void) { return 0; }
int sock_close(int s) { (void)s; return 0; }
int sock_send(int s, const void* buf, uint16_t len) { (void)s; (void)buf; (void)len; return 0; }
int sock_recv(int s, void* buf, uint16_t maxlen) { (void)s; (void)buf; (void)maxlen; return 0; }
