#!/bin/bash

IFACE="eth0"
MY_IP="172.20.177.47"

echo "=== Starting demo for mini-netstack ==="
echo "Please open another terminal and run:"
echo "    cd /mnt/c/miniNetStack && sudo ./netstack sniff"
read -p "Press ENTER once sniffer is running..."

echo "[1] Sending ARP request (may fail on loopback)..."
sudo arping -I $IFACE -c 1 $MY_IP

echo "[2] Sending ICMP ping..."
ping -c 1 $MY_IP

echo "[3] Sending UDP packet using netcat..."
echo "demo message" | nc -u -w1 $MY_IP 9999

echo "[4] Sending UDP packet using netstack CLI..."
sudo ../netstack udp-send $MY_IP "hi from demo.sh"

echo "Demo complete!"
