#include "icmp.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>

uint16_t checksum(void *data, int len) {
    uint16_t *buf = data;
    uint32_t sum = 0;

    for (; len > 1; len -= 2) sum += *buf++;
    if (len == 1) sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

void handle_icmp_packet(const uint8_t *packet, int length) {
    if (length < sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct icmphdr)) {
        return;
    }

    const struct ethhdr *eth = (const struct ethhdr *)packet;
    const struct iphdr *ip = (const struct iphdr *)(packet + sizeof(struct ethhdr));
    const struct icmphdr *icmp = (const struct icmphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));

    if (icmp->type != ICMP_ECHO) return;

    printf("ICMP Echo Request received from %s\n", inet_ntoa(*(struct in_addr *)&ip->saddr));

    // Build reply
    uint8_t reply[1500];
    struct ethhdr *eth_r = (struct ethhdr *)reply;
    struct iphdr *ip_r = (struct iphdr *)(reply + sizeof(struct ethhdr));
    struct icmphdr *icmp_r = (struct icmphdr *)(reply + sizeof(struct ethhdr) + sizeof(struct iphdr));
    int ip_len = length - sizeof(struct ethhdr);
    int total_len = sizeof(struct ethhdr) + ip_len;

    // MAC swap
    memcpy(eth_r->h_dest, eth->h_source, 6);
    uint8_t my_mac[6], my_ip[4];
    get_local_mac_ip("eth0", my_mac, my_ip);
    memcpy(eth_r->h_source, my_mac, 6);
    eth_r->h_proto = htons(ETH_P_IP);

    // IP header
    memcpy(ip_r, ip, ip_len);
    ip_r->saddr = ip->daddr;
    ip_r->daddr = ip->saddr;
    ip_r->check = 0;
    ip_r->check = checksum(ip_r, sizeof(struct iphdr));

    // ICMP header + data
    memcpy(icmp_r, icmp, ip_len - sizeof(struct iphdr));
    icmp_r->type = ICMP_ECHOREPLY;
    icmp_r->checksum = 0;
    icmp_r->checksum = checksum(icmp_r, ip_len - sizeof(struct iphdr));

    // Send
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    struct sockaddr_ll sa = {0};
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex("eth0");
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, eth_r->h_dest, 6);

    if (sendto(sockfd, reply, total_len, 0, (struct sockaddr *)&sa, sizeof(sa)) >= 0) {
        printf("✅ ICMP Echo Reply sent\n");
    } else {
        perror("sendto");
    }

    close(sockfd);
}
