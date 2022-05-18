# Comandos para configurar uma maquina RCOM
---
## In the begging of each class, clean the tuxs
---

Connect the TUXs on the switch that connects to the internet in the 5.1 socket.

Change between computers(Tuxys)
    1 - Double click on Scroll Lock
    2 - Click in the number of the tux that you want to select(2,3,4)
    3 - Press Enter

Inside the TUX
    1 - Execute the command in shell
    ~~~~ shell
    $ updateimage
    ~~~~

Em resumo: Conectar a rede do laboratorio, realizar a update e limpar os dispositivos, depois desconectar a rede

## EXPERIMENT 01
----------------

1. Disconnect switch from netlab and connect tux computers
2. Configure tuxy3 and tuxy4 using ifconfig and route commands
    - activar interface eth0
        root# ifconfig eth0 up
    - listar configurações actuais das interfaces de rede
        root# ifconfig

3. Register the IP and MAC addresses of network interfaces
    - tux3
        - configurar eth0 com endereço 172.16.5.1 e máscara 24 bits
            root# ifconfig eth0 172.16.5.1/24
        - IP: 172.16.5.1
        - MAC: 00:21:5A:61:2D:72
    - tux4
        - configurar eth0 com endereço 172.16.5.254 e máscara 24 bits
            root# ifconfig eth0 172.16.5.254/24
        - IP: 172.16.5.254
        - MAC: 00:21:5A:C3:78:70
4. Use ping command to verify connectivity between these computers [DONE]
    - tux3
        root# ping 172.16.5.254
    - tux4
        root# ping 172.16.5.1
5. Inspect forwarding (route –n) and ARP (arp –a) tables [DONE]
    TODO: Update with images what happened
6. Delete ARP table entries in tuxy1 (arp –d ipaddress) [DONE]
7. Start Wireshark in tuxy3.eth0 and start capturing packets []

8. In tuxy3, ping tuxy4 for a few seconds []

9. Stop capturing packets

10. Save log study it at home

» What are the ARP packets and what are they used for?
    The arp packets are used to resolve the name between the IP address and the MAC address making the correct association.
» What are the MAC and IP addresses of ARP packets and why?
    They are the MAC and IP adressess of the PC that are connected in the same network.
    In the request message we have the IP and MAC address from the Device that make the request, and the IP address that you want to find the MAC address associated. In the request message the field for the MAC address for the target is only 0s, in the response message this field is fullfilled with the correct MAC address.
» What packets does the ping command generate?
    In the wireshark generates packets from the ICMP protocol with a message for request and a message for reply.
» What are the MAC and IP addresses of the ping packets?
    The ones of the Source and Destination devices involved in the ping. Where the source is where the ping command are executed and the destination is the device that are beeing tested the connection, the endpoint.
» How to determine if a receiving Ethernet frame is ARP, IP, ICMP?
    From the field Type, where is indicated an Hexadecimal number witch indicates the type of protocol is beeing use.
    For example:
        If the type is 0x0800 is the ICMP and if is 0x0806 the type is the ARP protocol.
» How to determine the length of a receiving frame?
    Not sure by observation but I have a notion that is a value sended in the header of the frame.
» What is the loopback interface and why is it important?
    The loopback interface is usefull to keep the mac address of the device and to make tests in the own device if you want to test of implement a functionality, or to ping the own device to check some irregularities.

## EXPERIMENT 02
----------------

Tux net configuration 

    - tux2
        IP: 172.16.51.1/24
        MAC: 00:21:5A:61:2F:D6
    - tux3
        IP: 172.16.50.1/24
        MAC: 00:21:5A:61:2D:72
    - tux4
        IP: 172.16.50.254/24
        MAC: 00:21:5A:C3:78:70
    
Create vlany0 in the switch and add corresponding ports

- Connect the S0 of the tux3 to the T3 and the switch console T4 and inside the GKterm (/dev/ttyS0) access the switch and execute the commands:

First entrance on the switch:
~~~shell
tux-sw5>enable
password: 8nortel
~~~
Vlan 0 configuration
~~~shell
tux-sw5# configure terminal
tux-sw5# vlan 50
tux-sw5# end
tux-sw5# show vlan id 50
~~~
Vlan 1 configuration
~~~shell
tux-sw5# configure terminal
tux-sw5# vlan 50
tux-sw5# end
tux-sw5# show vlan id 50
~~~
Adding ports 4 e 5 to vlan 0
~~~shell
tux-sw5# configure terminal
tux-sw5# interface fastethernet 0/5
tux-sw5# switchport mode access
tux-sw5# switchport access vlan 50
tux-sw5# end
tux-sw5# show running-config interface fastethernet 0/5 
tux-sw5# show interfaces fastethernet 0/5 switchport

tux-sw5# configure terminal
tux-sw5# interface fastethernet 0/4
tux-sw5# switchport mode access
tux-sw5# switchport access vlan 50
tux-sw5# end
tux-sw5# show running-config interface fastethernet 0/4
tux-sw5# show interfaces fastethernet 0/4 switchport
~~~

Adding ports 8 to vlan 1
~~~shell
tux-sw5# configure terminal
tux-sw5# interface fastethernet 0/8
tux-sw5# switchport mode access
tux-sw5# switchport access vlan 51
tux-sw5# end
tux-sw5# show running-config interface fastethernet 0/8
tux-sw5# show interfaces fastethernet 0/8 switchport
~~~