#!/bin/bash

gcc -o main main.c speedtest.c initialize.c clock.c -lm -lwiringPi $(/usr/bin/python2.7-config --ldflags)

./main
