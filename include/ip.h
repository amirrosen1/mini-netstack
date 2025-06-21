#ifndef IP_H
#define IP_H

#include <stdint.h>

void parse_ip_packet(const uint8_t *packet, int length);

#endif
