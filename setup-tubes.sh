#!/bin/bash

# gpio map

# LEDS!
gpio mode 22 output # AM led
gpio mode 23 output # PM led

gpio write 22 1	# set both to on while booting ? 
gpio write 23 1

# on / off switch
gpio mode 21 input
gpio write 21 1	# enable pullup.

# enable HV suply
gpio mode 3 output
gpio write 3 0

# enable neions
gpio mode 2 output
gpio write 2 0

# set both ports on chip 1 to outputs
i2cset -y 1 0x20 0x00 0x00
i2cset -y 1 0x20 0x01 0x00

i2cset -y 1 0x21 0x00 0x00
i2cset -y 1 0x21 0x01 0x00	# needs changieng as last? 4 bits are inputs, or will be

#	chip 1
i2cset -y 1 0x20 0x13 0x88 # port A  # hours
i2cset -y 1 0x20 0x14 0x21 # port B  # minits

#	chip 2
i2cset -y 1 0x21 0x13 0x80 # port A . last? 4 bits are inputs
i2cset -y 1 0x21 0x14 0x11 # port B

# echo "tubes setup complete"
# echo " running clock program"

# cd ~/C-code
/home/pi/nixiclock/Nixi-clock2.0/main.out
