# mini-netstack

An educational project to build a mini TCP/IP stack in C using raw sockets on Linux. This project implements basic networking layers, including:

- Ethernet frame sniffer
- ARP request/reply handling
- ICMP echo responder (ping)
- UDP echo server and sender

## 📁 Structure

- `src/`: C source files (e.g., main.c, ethernet.c, arp.c)
- `include/`: Header files
- `kernel/`: Optional kernel module
- `docs/`: Documentation and diagrams
- `test/`: Test scripts and pcap files

## 🛠 Requirements

- Linux system (QEMU or Raspberry Pi supported)
- `gcc`, `make`, `tcpdump`, `wireshark`, `net-tools`
- Sudo access to open raw sockets

## ▶️ Getting Started

```bash
make
sudo ./netstack
