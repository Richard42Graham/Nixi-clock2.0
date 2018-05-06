#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <python2.7/Python.h>
#include <unistd.h>
#include <string.h>
#include "main.h"							// sotrore all the verable names,

void Initialize()
{
	wiringPiSetup();                                                // Setup the GPIO pins of the pi
	pinMode(amLed, OUTPUT);                                         // AM / PM light
	pinMode(pmLed, OUTPUT);                                         // AM / PM light
	pinMode(neons, OUTPUT);                                         // tube HV suply
	pinMode(highVoltagePowerSupply, OUTPUT);        		// Neons
	pinMode(onOffSwitch, INPUT);                            	// Switch for displays on or off

	digitalWrite(highVoltagePowerSupply, LOW);      		// Turn on the high voltage power supply
	digitalWrite(neons, LOW);                                       // Turn on the neons
	digitalWrite(amLed, LOW);                                       // Turn off AM/PM led
	digitalWrite(pmLed, LOW);                                       // Turn off AM/PM led
	digitalWrite(onOffSwitch, HIGH);                        	// Enables pullup on the on/off switch

	wiringPiI2CWriteReg8(hoursAndMinutesChipAddress, 0x00, 0x00); 	// set port A on chip 1 as all outputs
	wiringPiI2CWriteReg8(hoursAndMinutesChipAddress, 0x01, 0x00); 	// set port B on chip 1 as all ouptuts

	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChipAddress, 0x00, 0xF0); // set port A on the 2nd chip half input, half ouptut
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChipAddress, 0x01, 0x00); // set port B on the 2nd chip as all outputs
}

