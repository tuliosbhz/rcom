#Access
enable
8nortel

#Reset switch
#copy flash:mafalda_t5_switch running-config

configure terminal
vlan 50
end

configure terminal
interface fastethernet 0/4 #tux53
switchport mode access
switchport access vlan 50

interface fastethernet 0/5 #tux54 E0
switchport mode access
switchport access vlan 50
end

configure terminal
vlan 51
end

configure terminal
interface fastethernet 0/8 #tux52
switchport mode access
switchport access vlan 51
end

#Exp4 - Until Step 4
configure terminal
interface fastethernet 0/9 #tux54 E1
switchport mode access
switchport access vlan 51
end

configure terminal
interface fastethernet 0/20 #Router GE0/0
switchport mode access
switchport access vlan 51
end

#Router GE 0/1 -> 4.1 acima