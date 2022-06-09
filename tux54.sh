#Configure the eth0 interface in port 8 vlan50
ifconfig eth0 up 172.16.50.254/24

#Condfigure the eth1 interface in port 9 vlan51
ifconfig eth1 up 172.16.51.253/24

#The Rc(Router) como default router
route add default gw 172.16.51.254
#route add -net 172.16.0.0/16 gw 0.0.0.0
route add -net 172.16.20.0/24 gw 0.0.0.0

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts