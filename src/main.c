#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> // ETH_P_ALL
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip.h>     // for struct iphdr
#include <netinet/ip_icmp.h> // optional if using ICMP type in main
#include <netinet/udp.h>
#include "ethernet.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"


#define BUFFER_SIZE 65536

void sniff_packets() {
    int sockfd;
    uint8_t buffer[BUFFER_SIZE];

    // Create raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    printf("Sniffing packets... (press Ctrl+C to stop)\n");

    while (1) {
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (bytes_received < 0) {
            perror("recvfrom");
            break;
        }

        struct ethhdr *eth = (struct ethhdr *)buffer;
        uint16_t eth_type = ntohs(eth->h_proto);

        parse_ethernet_frame(buffer, bytes_received);  // Always print frame info

        if (eth_type == ETH_P_ARP) {
            // Pass just the ARP payload (after Ethernet header) to the ARP handler
            handle_arp_packet(buffer + sizeof(struct ethhdr),
                              bytes_received - sizeof(struct ethhdr));
        }

        if (eth_type == ETH_P_IP) {
            struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
            if (ip->protocol == IPPROTO_ICMP) {
                handle_icmp_packet(buffer, bytes_received);
            } else if (ip->protocol == IPPROTO_UDP) {
                handle_udp_packet(buffer, bytes_received);  // add this line
            }
        }
    }
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        printf("Commands: sniff, udp-send, arp-reply, ping-reply\n");
        return 1;
    }

    if (strcmp(argv[1], "sniff") == 0) {
        sniff_packets();
    } else if (strcmp(argv[1], "udp-send") == 0) {
        if (argc < 4) {
            printf("Usage: %s udp-send <target-ip> <message>\n", argv[0]);
            return 1;
        }
        udp_send(argv[2], argv[3]);  // ✅ call the actual UDP sending function
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
    }

    return 0;
}
