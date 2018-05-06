#!/bin/bash
echo Compiling...
gcc -o main.out main.c speedtest.c initialize.c clock.c -lm -lwiringPi $(/usr/bin/python2.7-config --ldflags)

if [ $? -eq 0 ]
then
	echo Runs...
	./main.out
else
	echo "Compile failed!";
fi
