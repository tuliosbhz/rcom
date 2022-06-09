#Configure the eth0 interface in port 8 vlan50
ifconfig eth0 up 172.16.51.1/24



#router mode: tux54 as default router
#IP tux54: 172.16.50.254
#route add -net 172.16.51.0/24 gw 172.16.50.254

#The Rc(Router) como default router
route add default gw 172.16.51.254
#route add -net 172.16.51.0/24 gw 0.0.0.0

echo 0 > /proc/sys/net/ipv4/ip_forward
echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/accept_redirects