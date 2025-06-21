#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>

void parse_ethernet_frame(const uint8_t *frame, int length);

#endif
