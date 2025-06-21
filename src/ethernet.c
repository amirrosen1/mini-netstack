#include "ethernet.h"
#include <stdio.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

void parse_ethernet_frame(const uint8_t *frame, int length) {
    struct ethhdr *eth = (struct ethhdr *)frame;

    uint16_t eth_type = ntohs(eth->h_proto);

    if (eth_type != ETH_P_ARP && eth_type != ETH_P_IP) {
        return;  // Ignore other types for now
    }

    printf("\nEthernet Frame (%d bytes):\n", length);
    printf("  Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
           eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("  Source MAC:      %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_source[0], eth->h_source[1], eth->h_source[2],
           eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("  EtherType:       0x%04x\n", eth_type);
}
