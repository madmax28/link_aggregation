#!/bin/bash

# Must be root
[ `id -u` == "0" ] || { echo "Must be root."; exit; }

# Necessary setup for iptables. Without this, the application
# will not be able to intercept the client application's traffic.
#
# This rule redirects any traffic designated to -d <ip>
# the netfilter queue <id>, from where the proxy application will retrieve
# the packets at a later time using the nfqueue library.
#
# NOTE: iptables must be set up before starting the client application

iptables -A OUTPUT -d 192.168.123.123 -j NFQUEUE --queue-num 0

# To remove:
#   iptables -t nat -D OUTPUT <num>
# To list:
#   iptables -t nat -L
