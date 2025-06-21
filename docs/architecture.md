# 🧠 mini-netstack Architecture

---

## 📚 Project Overview

**mini-netstack** is a lightweight, C-based network stack that manually handles low-level networking protocols in **user space** using raw sockets. It bypasses the standard Linux TCP/IP stack, providing educational insight into how networking works at the frame level.

✅ Built for **embedded Linux**, **WSL2**, or **QEMU**, this project is a hands-on systems networking portfolio piece.

---

## 📦 Protocol Layers Implemented

| Layer | Protocol | Function                            |
|-------|----------|-------------------------------------|
| L2    | Ethernet | Parse frames and dispatch by type   |
| L2    | ARP      | Resolve IP to MAC via ARP reply     |
| L3    | IPv4     | Handle IP headers and routing       |
| L4    | ICMP     | Respond to ping (Echo Request)      |
| L4    | UDP      | Echo messages or send manually      |

---

## 🧭 Protocol Packet Flows

### 🔁 ARP Resolution

```text
[ Host A ] -- ARP Request -->
             Who has 192.168.1.77?
                        ↓
     [ mini-netstack replies with MAC address ]
```


### 🛰 ICMP Echo (Ping)

```text
[ Host A ] -- ICMP Echo Request -->
             type=8, code=0
                        ↓
     [ mini-netstack crafts Echo Reply ]
             type=0 (Echo Reply)
             ← Sent back to Host A
```

### 📦 UDP Echo Server

```text
[ Host A ] -- UDP packet -->
             dst port: 9999
             payload: "hello"
                        ↓
     [ mini-netstack replies with same payload ]
             ← src/dst ports and IPs swapped
```

### 📤 UDP CLI Sender (udp-send)

```text
./netstack udp-send <target-ip> "hi there"

Internally:
    → Manually builds Ethernet + IP + UDP headers
    → Sends via raw socket (AF_PACKET)
```

### ⚙️ Internal Architecture

                     +----------------------+
                     |    Raw Socket (RX)   |
                     | AF_PACKET, SOCK_RAW  |
                     +----------+-----------+
                                |
               +-----------------------------+
               |                             |
         +-----v-----+             +---------v---------+
         | Ethernet  |             |  Parse EtherType  |
         |   Frame   |             +-------------------+
         +-----+-----+                        |
               |                  +----------+----------+
               |                  |                     |
          +----v---+         +----v----+           +-----v-----+
          |  ARP   |         |   IP    |           |  Unknown   |
          +----+---+         +----+----+           +-----------+
               |                  |
         +-----v-----+      +-----v------+
         | ARP Reply |      | Check Protocol (ICMP/UDP) |
         +-----------+      +---------------------------+


### 📂 Code Structure
```text
mini-netstack/
├── src/
│   ├── main.c          # CLI entry + sniff loop
│   ├── ethernet.c/.h   # Ethernet frame parser
│   ├── arp.c/.h        # ARP request/response
│   ├── ip.c/.h         # IP header parser
│   ├── icmp.c/.h       # ICMP Echo responder
│   ├── udp.c/.h        # UDP echo + sender
│   └── utils.c/.h      # MAC/IP lookup, checksum
├── test/
│   ├── demo.sh         # Automated test script
│   └── demo_capture.pcap
├── docs/
│   └── architecture.md # This file
├── netstack            # Compiled binary
├── Makefile
└── README.md
```


### 🧪 Tools Used
```text
| Tool                 | Purpose                                           |
|----------------------|---------------------------------------------------|
| `tcpdump`            | Capture and save raw network traffic (.pcap)      |
| `Wireshark`          | Visually inspect and analyze network packets      |
| `ping`               | Test ICMP Echo request/reply functionality        |
| `arping`             | Send ARP requests to trigger ARP handler          |
| `netcat (nc)`        | Send UDP payloads to test echo server (port 9999) |
| `iperf` *(optional)* | Generate UDP traffic for performance testing      |
| `asciinema` / `peek` | Record terminal sessions or animated GIF demos    |
```

### 🧠 Future Ideas
```text
🔐 Add firewall or NAT logic

📡 Support DNS parsing

📦 Recognize TCP SYN, ACK (basic TCP state logic)

🧱 Add kernel module using Netfilter hooks

📊 Build a browser-based packet visualizer UI
```

### 🏁 Final Notes
```text
This project demonstrates:

✅ Mastery of C and raw socket programming
✅ Hands-on implementation of Ethernet, ARP, ICMP, UDP
✅ Deep understanding of packet-level networking
✅ Professional use of Linux tools and embedded debugging
✅ Excellent material for interviews and GitHub portfolios
```