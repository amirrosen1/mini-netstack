#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int get_local_mac_ip(const char *iface, uint8_t *mac, uint8_t *ip);

#endif
