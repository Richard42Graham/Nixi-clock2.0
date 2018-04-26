#!/bin/sh
gcc clock.c -lm -lwiringPi $(/usr/bin/python2.7-config --ldflags)

# ./a.out
