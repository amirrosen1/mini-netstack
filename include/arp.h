#ifndef ARP_H
#define ARP_H

#include <stdint.h>

void handle_arp_packet(const uint8_t *packet, int length);

#endif
