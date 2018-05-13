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

struct SpeedTestData
{
	int hoursAndMinutesChip;
	int secondsAndMicrosecondsAndModeChip;
};

void SpeedTest(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip);
int SpeedTestEnter(void* d);
int SpeedTestUpdate(void* d);
int SpeedTestExit(void* d);

struct Nixi_State CreateSpeedTestState(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip)
{
	struct SpeedTestData *data = malloc(sizeof(struct SpeedTestData));
	data->hoursAndMinutesChip = hoursAndMinutesChip;
	data->secondsAndMicrosecondsAndModeChip = secondsAndMicrosecondsAndModeChip;

	struct Nixi_State speedTestState;
	speedTestState.Enter = &SpeedTestEnter;
	speedTestState.Update = &SpeedTestUpdate;
	speedTestState.Exit = &SpeedTestExit;
	speedTestState.Data = (void*)data;
	return speedTestState;
}

int SpeedTestEnter(void* d)
{
	struct SpeedTestData* data = (struct SpeedTestData*)d;
	DisplayNumber(data->hoursAndMinutesChip,
		data->secondsAndMicrosecondsAndModeChip,
		9);
	SpeedTest(
		data->hoursAndMinutesChip,
		data->secondsAndMicrosecondsAndModeChip);
	return 0;
}

int SpeedTestUpdate(void* d)
{
	return 0;
}

int SpeedTestExit(void* d)
{
	return 0;
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

