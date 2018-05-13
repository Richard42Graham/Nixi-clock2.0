#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "speedtest.h"
#include "clock.h"
#include "placeHolderState.h"

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

	struct Nixi_State clock12HState = CreateClockState(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 0);
	struct Nixi_State clock24HState = CreateClockState(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 1);
	struct Nixi_State speedTestState = CreateSpeedTestState(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip);
	struct Nixi_State placeHolderState = CreatePlaceHolderState(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 0);
	struct Nixi_State placeHolder2State = CreatePlaceHolderState(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 2);
	
	struct Nixi_State currentState = clock12HState;
	currentState.Enter(currentState.Data);
	char lastMode = 0x00;
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

		currentState.Update(currentState.Data);

		char modeSwitch = (wiringPiI2CReadReg8(secondsAndMicrosecondsAndModeChip, chipPortA) & 0x0F);
		if (lastMode != modeSwitch)
		{
			lastMode = modeSwitch;
			currentState.Exit(currentState.Data);

			switch (modeSwitch) {
			case 0x00:
			{
				currentState = clock12HState;
				break;
			}
			case 0x01:
			{
				currentState = clock24HState;
				break;
			}
			case 0x02:
			{
				currentState = placeHolderState;
				break;
			}
			case 0x04:
			{
				currentState = placeHolder2State;
				break;
			}
			case 0x08:
			{
				currentState = speedTestState;
				break;
			}
			}
			currentState.Enter(currentState.Data);
		}
	}
}

void DisplayNumber(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number)
{
	//Set all the tubes to show 0
	wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortA, CaculateTime(number, hoursBits));
	wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortB, CaculateTime(number, minutsBits));
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortA, CaculateTime(number, secondsBits));
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortB, CaculateTime(number, deciseconds));
}

char CaculateTime(int number, char map[2][10]) {
	if (number > 9) {
		char out = (map[1][number % 10] << 4) & 0xFF;
		out = out | map[0][(number / 10) % 10];
		return out;
	}
	else {
		char out = (map[1][number] << 4) & 0xFF;
		out = out | map[0][0];
		return out;
	}
}




