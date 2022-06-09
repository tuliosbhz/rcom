# Questions on each experiment RCOM
----

## EXPERIMENT 01
----------------

» What are the ARP packets and what are they used for?

- The arp packets are used to resolve the name between the IP address and the MAC address making the correct association.
» What are the MAC and IP addresses of ARP packets and why?

- They are the MAC and IP adressess of the PC that are connected in the same network.
    In the request message we have the IP and MAC address from the Device that make the request, and the IP address that you want to find the MAC address associated. In the request message the field for the MAC address for the target is only 0s, in the response message this field is fullfilled with the correct MAC address.

» What packets does the ping command generate?

- In the wireshark generates packets from the ICMP protocol with a message for request and a message for reply.

» What are the MAC and IP addresses of the ping packets?

- The ones of the Source and Destination devices involved in the ping. Where the source is where the ping command are executed and the destination is the device that are beeing tested the connection, the endpoint.

» How to determine if a receiving Ethernet frame is ARP, IP, ICMP?

- From the field Type, where is indicated an Hexadecimal number witch indicates the type of protocol is beeing use.
- For example:
        If the type is 0x0800 is the ICMP and if is 0x0806 the type is the ARP protocol.

» How to determine the length of a receiving frame?

- Not sure by observation but I have a notion that is a value sended in the header of the frame.

» What is the loopback interface and why is it important?

- The loopback interface is usefull to keep the mac address of the device and to make tests in the own device if you want to test of implement a functionality, or to ping the own device to check some irregularities.

## EXPERIMENT 02
----------------

1. How to configure vlan50?
    First we connect the serial port of the PC in the switch serial port for configuration
    At the GtkTerm we press enter and after that is just to execute the commands as is showed bellow:
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
2. How many broadcast domains are there? How can you conclude it from the logs?
    - Broadcast domains are the devices connected in a Layer 2. So every device that has an interface connected to each other can find using ARP anyone with a broadcast command.

## EXPERIMENT 03
----------------

1. What routes are there in the tuxes? What are their meaning?
2. What information does an entry of the forwarding table contain?
3. What ARP messages, and associated MAC addresses, are observed and why?
4. What ICMP packets are observed and why?
5. What are the IP and MAC addresses associated to ICMP packets and why?

## EXPERIMENT 04
------------------

1. How to configure a static route in a commercial router?
2. What are the paths followed by the packets in the experiments carried out and why?
3. How to configure NAT in a commercial router ?
4. What does NAT do?