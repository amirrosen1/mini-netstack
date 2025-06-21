#include "arp.h"
#include <stdio.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include "utils.h"


void handle_arp_packet(const uint8_t *packet, int length) {
    if (length < sizeof(struct ether_arp)) {
        printf("ARP packet too short\n");
        return;
    }

    struct ether_arp *arp = (struct ether_arp *)packet;

    if (ntohs(arp->ea_hdr.ar_op) != ARPOP_REQUEST) return;

    char sender_ip[INET_ADDRSTRLEN];
    char target_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, arp->arp_spa, sender_ip, sizeof(sender_ip));
    inet_ntop(AF_INET, arp->arp_tpa, target_ip, sizeof(target_ip));

    printf("ARP Request:\n");
    printf("  Who has %s? Tell %s\n", target_ip, sender_ip);

    // Step 1: Get local MAC and IP
    uint8_t my_ip[4];
    uint8_t my_mac[6];
    if (get_local_mac_ip("eth0", my_mac, my_ip) < 0) {
        perror("get_local_mac_ip");
        return;
    }

    // Step 2: Check if ARP request is for us
    if (memcmp(arp->arp_tpa, my_ip, 4) != 0) return;

    printf("➡  Responding to ARP for my IP: %s\n", target_ip);

    // Step 3: Build ARP reply packet (Ethernet + ARP)
    uint8_t reply[42]; // 14 + 28
    struct ethhdr *eth = (struct ethhdr *)reply;
    struct ether_arp *arp_reply = (struct ether_arp *)(reply + 14);

    // Ethernet header
    memcpy(eth->h_dest, arp->arp_sha, 6);        // Target = sender
    memcpy(eth->h_source, my_mac, 6);            // Source = us
    eth->h_proto = htons(ETH_P_ARP);

    // ARP payload
    arp_reply->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arp_reply->ea_hdr.ar_pro = htons(ETH_P_IP);
    arp_reply->ea_hdr.ar_hln = 6;
    arp_reply->ea_hdr.ar_pln = 4;
    arp_reply->ea_hdr.ar_op  = htons(ARPOP_REPLY);

    memcpy(arp_reply->arp_sha, my_mac, 6);       // Our MAC
    memcpy(arp_reply->arp_spa, my_ip, 4);        // Our IP
    memcpy(arp_reply->arp_tha, arp->arp_sha, 6); // Their MAC
    memcpy(arp_reply->arp_tpa, arp->arp_spa, 4); // Their IP

    // Step 4: Send it using raw socket
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    struct sockaddr_ll sa = {0};
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = if_nametoindex("eth0");
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, arp->arp_sha, 6); // destination MAC

    if (sendto(sockfd, reply, sizeof(reply), 0,
               (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto");
    } else {
        printf("ARP reply sent to %s\n", sender_ip);
    }

    close(sockfd);
}

