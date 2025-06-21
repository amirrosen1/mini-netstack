#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "utils.h"

int get_local_mac_ip(const char *iface, uint8_t *mac, uint8_t *ip) {
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    // Get MAC address
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) return -1;
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

    // Get IP address
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1) return -1;
    struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    memcpy(ip, &ipaddr->sin_addr, 4);

    close(sockfd);
    return 0;
}
