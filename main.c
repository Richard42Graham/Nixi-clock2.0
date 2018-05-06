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

#include "main.h"
#include "speedtest.h"
#include "clock.h"

// hours                                                // first digit                                          // second digit
char hoursBits[2][10] = { {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09}, {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09} };
//                          -0   -1     -2    -3     -4    -5    -6    -7    -8    -9  ||  0-    1-    2-    3-    4-    5-    6-    7-    8-    9-

//Mins
char minutsBits[2][10] = { {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09} };

//Sec
char secondsBits[2][10] = { {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09},{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09} };

//decisecond                                          10th's of a second                               not used, 4 bit mode switch here.
char deciseconds[2][10] = { {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

const int onOffSwitch = 21;
const int amLed = 23;
const int pmLed = 22;
const int neons = 2;
const int highVoltagePowerSupply = 3;

// i2c port expander usefull adresses.
const int hoursAndMinutesChipAddress = 0x20;
const int secondsAndMicrosecondsAndModeChipAddress = 0x21;
const int chipPortA = 0x13;
const int chipPortB = 0x14;


int main(int argc, char *argv[]) {

	// first chip
	int hoursAndMinutesChip = wiringPiI2CSetup(hoursAndMinutesChipAddress);

	// Second Chip
	int secondsAndMicrosecondsAndModeChip = wiringPiI2CSetup(secondsAndMicrosecondsAndModeChipAddress);
	Initialize();

	int neonBlinkOrientation = GetNeonBlinkOrientation();

	while (1)
	{
		// sleep for 50 milliSeconds
		usleep(100 * 500);

		// Show no light if switch is pressed 
		if (digitalRead(onOffSwitch) == HIGH)
		{
			// Turn off HV
			digitalWrite(highVoltagePowerSupply, HIGH);

			// Turn off neons
			digitalWrite(neons, LOW);

			// turn off am and pm lights
			digitalWrite(amLed, LOW);
			digitalWrite(pmLed, LOW);
			continue;
		}

		// Turn on high voltage power supply
		digitalWrite(highVoltagePowerSupply, LOW);


		char modeSwitch = (wiringPiI2CReadReg8(secondsAndMicrosecondsAndModeChip, chipPortA) & 0x0F);

		switch (modeSwitch) {
		case 0x00:
		{
			DisplayTime(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 0, neonBlinkOrientation);
			break;
		}
		case 0x01:
		{
			DisplayTime(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 1, neonBlinkOrientation);
			break;
		}
		case 0x02:
		{
			DisplayNumber(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 0);
			break;
		}
		case 0x04:
		{
			DisplayNumber(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 0);
			break;
		}
		case 0x08:
		{
			SpeedTest(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip);
			break;
		}
		}
	}
}

