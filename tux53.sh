#Configure the eth0 interface in port 8 vlan50
ifconfig eth0 up 172.16.50.1/24

#router mode: tux54 as default router
route add default gw 172.16.50.254