#!/bin/bash

ip link add name br0 type bridge
ip link set eth0 down
ip addr flush dev eth0
ip tuntap add name tap0 mode tap
ip link set eth0 master br0
ip link set tap0 master br0
ip link set eth0 up
ip link set tap0 up
dhclient -v br0
