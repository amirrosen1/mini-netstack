#ifndef UDP_H
#define UDP_H

#include <stdint.h>

void handle_udp_packet(const uint8_t *packet, int length);
void udp_send(const char *target_ip_str, const char *message);

#endif
