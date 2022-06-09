# RCOM - Configure Network
---------------------------
## Configuração dos PCs (Tux50)

A bancada de trabalho do nosso grupo é a 5. O que substitui o y no guião dado.

1. Conectar os tux52, tux53 e tux54 na internet (5.1)
2. Executar o comando no terminal updateimage em cada PC(tux5x)
    ~~~shell
    $ updateimage 
    ~~~
3. Resetar o switch e o rooteador
    - Conectar fisicamente a interface S0 de algum Tux ao Switch/Rooter Console
    - Dentro do GTKterm executar comandos de reset impressos na mesa da bancada do laboratório
    ~~~shell
    tux-rtr5# enable
    tux-rtr5# copy flash:tux5-clean startup-config
    tux-rtr5# reload
    ~~~
    Reset switch:
    ~~~shell
    tux-sw5# enable
    tux-sw5# configure terminal
    tux-sw5#(config) no vlan 2-4094
    tux-sw5#(config) exit
    tux-sw5# copy flash:tux5-clean startup-config
    reload 
    ~~~
4.  Configurar os ips de cada maquina segundo com os endereços de rede desejados:
    - activar interface eth0
        ~~~shell
        root# ifconfig eth0 up
        ~~~
    - listar configurações actuais das interfaces de rede
        ~~~shell
        root# ifconfig <interface> <ip address/24>
        ~~~
5. Configure the VLans in the switch
- Enter the switch
~~~shell
tux-sw5>enable
password: 8nortel
~~~
- Create Vlan in the switch with the commands
- Vlan 50 configuration
~~~shell
tux-sw5# configure terminal
tux-sw5# vlan 50
tux-sw5# end
tux-sw5# show vlan id 50
~~~
- Add ports to the Vlans create so can be accessed by cables
- Adding ports 5 to vlan 0
~~~shell
tux-sw5# configure terminal
tux-sw5# interface fastethernet 0/5
tux-sw5# switchport mode access
tux-sw5# switchport access vlan 50
tux-sw5# end
tux-sw5# show running-config interface fastethernet 0/5 
~~~

6. Configuration of the Router
    1. Configure the IP address of the router in the VLAN that you want to