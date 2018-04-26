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

// hours                                                // first digit                                          // second digit
char hoursBits[2][10] = { {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09}, {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09} };
//                          -0   -1     -2    -3     -4    -5    -6    -7    -8    -9  ||  0-    1-    2-    3-    4-    5-    6-    7-    8-    9-

//Mins
char minutsBits[2][10] = { {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09} };

//Sec
char secondsBits[2][10] = { {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09},{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09} };

//decisecond                                          10th's of a second                               not used, 4 bit mode switch here.
char deciseconds[2][10] = { {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

// commonly used pins, labled so its easyer to read my code.

//The on/off switch
//By default it should be pulled up high
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

//Function declarations
char CaculateTime(int number, char map[2][10]);
void Initialize();
void DisplayTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation);
char GetHours(struct tm *tm_p, int use24h);
char GetDecisecond(struct timeval tv);
void DisplayNumber(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number);
int GetNeonBlinkOrientation();
void SpeedTest(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip);

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

void Initialize()
{
	wiringPiSetup();							// Setup the GPIO pins of the pi
	pinMode(amLed, OUTPUT);						// AM / PM light
	pinMode(pmLed, OUTPUT);						// AM / PM light
	pinMode(neons, OUTPUT);						// tube HV suply
	pinMode(highVoltagePowerSupply, OUTPUT);	// Neons
	pinMode(onOffSwitch, INPUT);				// Switch for displays on or off

	digitalWrite(highVoltagePowerSupply, LOW);	// Turn on the high voltage power supply 
	digitalWrite(neons, LOW);					// Turn on the neons 
	digitalWrite(amLed, LOW);					// Turn off AM/PM led
	digitalWrite(pmLed, LOW);					// Turn off AM/PM led
	digitalWrite(onOffSwitch, HIGH);			// Enables pullup on the on/off switch

	wiringPiI2CWriteReg8(hoursAndMinutesChipAddress, 0x00, 0x00); // set port A on chip 1 as all outputs
	wiringPiI2CWriteReg8(hoursAndMinutesChipAddress, 0x01, 0x00); // set port B on chip 1 as all ouptuts

	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChipAddress, 0x00, 0xF0); // set port A on the 2nd chip half input, half ouptut
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChipAddress, 0x01, 0x00); // set port B on the 2nd chip as all outputs
}

void DisplayNumber(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number)
{
	//Set all the tubes to show 0
	wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortA, CaculateTime(number, hoursBits));
	wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortB, CaculateTime(number, minutsBits));
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortA, CaculateTime(number, secondsBits));
	wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortB, CaculateTime(number, deciseconds));
}

//void Rince(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number)
//{
//	switch (counter )
//		{
//			case 0:
//				{
//					wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortA, 42);
					// counter ++;
//				} break;
//			case 1:
//				{
					// write 1's
					// counter ++;
//				} break;
//		}
//}

void DisplayTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation)
{
	//variables for display time
	struct tm *tm_p;
	time_t current_time;
	struct timeval tv;

	current_time = time(NULL);
	tm_p = localtime(&current_time);
	gettimeofday(&tv, NULL);

	char hours = GetHours(tm_p, use24h);
	char mins = CaculateTime(tm_p->tm_min, minutsBits);
	char sec = CaculateTime(tm_p->tm_sec, secondsBits);
	char decisec = GetDecisecond(tv);
	int enableNeons = 0;
	if (tm_p->tm_sec % 2 == neonBlinkOrientation)
	{
		enableNeons = HIGH;
	}
	else
	{
		enableNeons = LOW;
	}

	//AM / PM leds
	if (use24h)
	{
		digitalWrite(amLed, LOW);
		digitalWrite(pmLed, LOW);
	}
	else if (tm_p->tm_hour < 12) {
		digitalWrite(amLed, HIGH);
		digitalWrite(pmLed, LOW);
	}
	else {
		digitalWrite(amLed, LOW);
		digitalWrite(pmLed, HIGH);
	}

	//Neons 
	digitalWrite(neons, enableNeons);

	//Hours
	if (wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortA, hours) == -1) {

		printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
	}

	//Minutes
	if (wiringPiI2CWriteReg8(hoursAndMinutesChip, chipPortB, mins) == -1) {

		printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
	}

	//deciseconds
	if (wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortA, ((sec & 0x0F) << 4)) == -1) {

		printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
	}

	//Second
	if (wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortB, (((sec >> 4) & 0x0F) | ((decisec) & 0xF0))) == -1) {

		printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
	}

}


char GetDecisecond(struct timeval tv) {
	// Round to nearest decisecond
	int decisecond = lrint(tv.tv_usec / 100000.0);

	// Allow for rounding up to nearest second
	if (decisecond >= 999) {
		decisecond -= 999;
		tv.tv_sec++;
	}

	if (decisecond == 10) {
		decisecond = 0;
	}
	return CaculateTime(decisecond, deciseconds);
}

char GetHours(struct tm *tm_p, int use24h) {
	int hour = tm_p->tm_hour;
	if (use24h != 1) {
		if (hour > 12) {
			hour = hour - 12;
		}
	}
	return CaculateTime(hour, hoursBits);
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

//Gay vs straight
int GetNeonBlinkOrientation() {
	struct tm *tm_p;
	time_t current_time;
	struct timeval tv;
	current_time = time(NULL);
	tm_p = localtime(&current_time);
	gettimeofday(&tv, NULL);

	if (tm_p->tm_sec % 2 == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void SpeedTest(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip)
{
	printf("Starting speed test...\n");
	fflush(stdout);
	//DisplayNumber(hoursAndMinutesChip, secondsAndMicrosecondsAndModeChip, 99);

	FILE* file;
	Py_SetProgramName("speedtest.py");
	Py_Initialize();
	file = fopen("/home/pi/C-code/speedtest.py", "r");
	PyRun_SimpleFile(file, "/home/pi/C-code/speedtest.py");
	Py_Finalize();

	printf("Speed Caculated\n");
	fflush(stdout);
	//      display animation on the tubes here

	// ??  wiringPiI2CWriteReg8(0x20, 0x14, 0xFF);

	FILE *fp;
	// posibly scroll the first result
	fp = fopen("/home/pi/C-code/speed.txt", "r");
	// display the 2 results an animate
	if (fp == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	size_t buffer_size = 80;
	char *buffer = malloc(buffer_size * sizeof(char));

	int lineNumber = 0;
	while (-1 != getline(&buffer, &buffer_size, fp))
	{
		switch (lineNumber) {
		case 0: {

			int pingSpeed = (int)atof(buffer);
			int length = 3;
			int i, j;
			char pingFirst, pingSec;
			for (i = 1; i < 4; i++) {

				switch (i) {
				case 1: {
					pingFirst = CaculateTime((pingSpeed) % 10, deciseconds);
					break;
				}

				case 2: {
					j = (pingSpeed / 10) % 10;
					break;
				}

				case 3: {
					int last = (pingSpeed / 100) % 10;
					j = j + (last * 10);
					pingSec = CaculateTime(j, secondsBits);
					break;
				}
				}
			}

			if (wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, 0x14, (((pingSec >> 4) & 0x0F) | ((pingFirst) & 0xF0))) == -1) {
				printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
			}

			if (wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, 0x13, ((pingSec & 0x0F) << 4)) == -1) {
				printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
			}
			break;
		}
		case 1: {
			float downloadSpeed = atof(buffer);
			//                                      printf("%f\n", downloadSpeed);
			if (downloadSpeed < 100) {
				char down = CaculateTime((int)downloadSpeed, hoursBits);
				if (wiringPiI2CWriteReg8(hoursAndMinutesChip, 0x13, down) == -1) {
					printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
				}
			}
			break;
		}
		case 2: {
			float uploadSpeed = atof(buffer);
			//                                      printf("%f\n", uploadSpeed);
			if (uploadSpeed < 100) {
				char up = CaculateTime((int)uploadSpeed, minutsBits);
				if (wiringPiI2CWriteReg8(hoursAndMinutesChip, 0x14, up) == -1) {
					printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
				}
			}
			break;
		}
		}
		lineNumber++;
	}
	//              animat the results by flashing ? and repeat it again.
	fclose(fp);
	fflush(stdout);
}
