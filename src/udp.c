#include "udp.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/if.h>

#define UDP_PORT 9999

void handle_udp_packet(const uint8_t *packet, int length) {
    const struct ethhdr *eth = (const struct ethhdr *)packet;
    const struct iphdr *ip = (const struct iphdr *)(packet + sizeof(struct ethhdr));
    const struct udphdr *udp = (const struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    const uint8_t *payload = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    int payload_len = ntohs(udp->len) - sizeof(struct udphdr);

    if (ntohs(udp->dest) != UDP_PORT) return;

    char src_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip->saddr, src_ip, sizeof(src_ip));
    printf("UDP packet received from %s:%d → payload: %.*s\n",
        src_ip, ntohs(udp->source), payload_len, payload);

    // Prepare response
    uint8_t reply[1500];
    struct ethhdr *eth_r = (struct ethhdr *)reply;
    struct iphdr *ip_r = (struct iphdr *)(reply + sizeof(struct ethhdr));
    struct udphdr *udp_r = (struct udphdr *)(reply + sizeof(struct ethhdr) + sizeof(struct iphdr));
    uint8_t *payload_r = reply + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);

    uint8_t my_mac[6], my_ip[4];
    get_local_mac_ip("eth0", my_mac, my_ip);

    memcpy(eth_r->h_dest, eth->h_source, 6);
    memcpy(eth_r->h_source, my_mac, 6);
    eth_r->h_proto = htons(ETH_P_IP);

    memcpy(ip_r, ip, sizeof(struct iphdr));
    ip_r->saddr = ip->daddr;
    ip_r->daddr = ip->saddr;
    ip_r->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len);
    ip_r->check = 0;
    ip_r->check = checksum(ip_r, sizeof(struct iphdr));

    memcpy(udp_r, udp, sizeof(struct udphdr));
    udp_r->source = udp->dest;
    udp_r->dest = udp->source;
    udp_r->len = htons(sizeof(struct udphdr) + payload_len);
    udp_r->check = 0;  // Optional: skip UDP checksum

    memcpy(payload_r, payload, payload_len);

    int total_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len;

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    struct sockaddr_ll sa = {0};
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex("eth0");
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, eth_r->h_dest, 6);

    if (sendto(sockfd, reply, total_len, 0, (struct sockaddr *)&sa, sizeof(sa)) >= 0) {
        printf("✅ UDP Echo reply sent to %s:%d\n", src_ip, ntohs(udp->source));
    } else {
        perror("sendto");
    }

    close(sockfd);
}

void udp_send(const char *target_ip_str, const char *message) {
    uint8_t frame[1500];

    struct ethhdr *eth = (struct ethhdr *)frame;
    struct iphdr *ip = (struct iphdr *)(frame + sizeof(struct ethhdr));
    struct udphdr *udp = (struct udphdr *)(frame + sizeof(struct ethhdr) + sizeof(struct iphdr));
    uint8_t *payload = frame + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    int payload_len = strlen(message);

    // Fill payload
    memcpy(payload, message, payload_len);

    // Local info
    uint8_t my_mac[6], my_ip[4];
    get_local_mac_ip("eth0", my_mac, my_ip);

    // Target info
    uint8_t target_ip[4];
    inet_pton(AF_INET, target_ip_str, target_ip);

    // MAC dest (broadcast as placeholder)
    memset(eth->h_dest, 0xff, 6);
    memcpy(eth->h_source, my_mac, 6);
    eth->h_proto = htons(ETH_P_IP);

    // IP header
    memset(ip, 0, sizeof(struct iphdr));
    ip->version = 4;
    ip->ihl = 5;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = *(uint32_t *)my_ip;
    ip->daddr = *(uint32_t *)target_ip;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len);
    ip->check = checksum(ip, sizeof(struct iphdr));

    // UDP header
    udp->source = htons(12345);
    udp->dest = htons(9999);
    udp->len = htons(sizeof(struct udphdr) + payload_len);
    udp->check = 0; // skipping checksum

    int total_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len;

    // Send
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    struct sockaddr_ll sa = {0};
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex("eth0");
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, eth->h_dest, 6);

    if (sendto(sockfd, frame, total_len, 0, (struct sockaddr *)&sa, sizeof(sa)) >= 0) {
        printf("✅ UDP packet sent to %s:9999\n", target_ip_str);
    } else {
        perror("sendto");
    }

    close(sockfd);
}
