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
#include "clock.h"

struct ClockData
{
	int hoursAndMinutesChip;
	int secondsAndMicrosecondsAndModeChip;
	int use24h;
	int neonBlinkOrientation;
};

int ClockEnter(void* d);
int ClockUpdate(void* d);
int ClockExit(void* d);
void DisplayTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation);
char GetHours(struct tm *tm_p, int use24h);
char GetDecisecond(struct timeval tv);
int GetNeonBlinkOrOrEven();

struct Nixi_State CreateClockState(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h)
{
	struct ClockData *data = malloc(sizeof(struct ClockData));
	data->hoursAndMinutesChip = hoursAndMinutesChip;
	data->secondsAndMicrosecondsAndModeChip = secondsAndMicrosecondsAndModeChip;
	data->use24h = use24h;
	data->neonBlinkOrientation = GetNeonBlinkOrOrEven();

	struct Nixi_State clockState;
	clockState.Enter = &ClockEnter;
	clockState.Update = &ClockUpdate;
	clockState.Exit = &ClockExit;
	clockState.Data = (void*) data;
	return clockState;
}

int ClockEnter(void* d)
{
	struct ClockData* data = (struct ClockData*)d;
	data->neonBlinkOrientation = GetNeonBlinkOrOrEven();
	return 0;
}

int ClockUpdate(void* d)
{
	struct ClockData* data = (struct ClockData*)d;
	DisplayTime(
		data->hoursAndMinutesChip, 
		data->secondsAndMicrosecondsAndModeChip, 
		data->use24h, 
		data->neonBlinkOrientation);

	return 0;
}

int ClockExit(void* d)
{
	return 0;
}

//Gay vs straight
int GetNeonBlinkOrOrEven() {
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