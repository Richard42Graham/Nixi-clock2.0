#!/bin/bash

# set chip 1 set bothports A and B to outputs
i2cset -y 1 0x20 0x00 0x00
i2cset -y 1 0x20 0x01 0x00

# set chip 2, port A part input, part output. Port B all outputs
i2cset -y 1 0x21 0x00 0x80
i2cset -y 1 0x21 0x01 0x00

# display 0 on all tubes
#	set to 0 --------------\
i2cset -y 1 0x20 0x13 0x88
i2cset -y 1 0x20 0x14 0x21
#				|
i2cset -y 1 0x21 0x13 0x80
i2cset -y 1 0x21 0x14 0x11
#	set to 0 --------------/

# whil true bash loop
y="1"
while [ $y -lt 10 ]
do

i2cget -y 1 0x20 0x13
i2cget -y 1 0x20 0x14

i2cget -y 1 0x21 0x13	# bits of intrest :P
i2cget -y 1 0x21 0x14

# read i2c port
#print result here?
sleep 0.1
clear
done
