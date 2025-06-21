#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>

void handle_icmp_packet(const uint8_t *packet, int length);

#endif
