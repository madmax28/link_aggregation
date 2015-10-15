#!/bin/bash
#
# Necessary setup for iptables. Without this, the application
# will not be able to intercept the client application's traffic.
#
# This rule redirects any traffic designated to -d <ip>
# to the local host. The IP header's destination address
# is changed to 127.0.0.1
#
# The used port is preserved
#
# Note: iptables must be set up before starting the client application

iptables -t nat -A OUTPUT -d 127.0.10.10 -j REDIRECT

# To remove:
#   iptables -t nat -D OUTPUT <num>
# To list:
#   iptables -t nat -L
