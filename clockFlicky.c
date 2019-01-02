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
#include "clockFlicky.h"

int minimumEffectLength = 5;		// in ms (milli seconds)	MIN
int maximumEffectLength = 30;		// in ms 			MAX

int minimumTimeToNextEffect = 1000;
int maximumTimeToNextEffect = 10000;

struct ClockFlickyData
{
	int hoursAndMinutesChip;
	int secondsAndMicrosecondsAndModeChip;
	int use24h;
	int neonBlinkOrientation;
	unsigned long long nextEffectTime;
	int effectLength;
};

int ClockFlickyEnter(void* d);
int ClockFlickyUpdate(void* d);
int ClockFlickyExit(void* d);
void DisplayFlickyTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation);
char GetFlickyHours(struct tm *tm_p, int use24h);
char GetFlickyDecisecond(struct timeval tv);
int GetFlickyNeonBlinkOrOrEven();
unsigned long long GetFlickyUnixTime();

struct Nixi_State CreateClockFlickyState(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h)
{
	struct ClockFlickyData *data = malloc(sizeof(struct ClockFlickyData));
	data->hoursAndMinutesChip = hoursAndMinutesChip;
	data->secondsAndMicrosecondsAndModeChip = secondsAndMicrosecondsAndModeChip;
	data->use24h = use24h;
	data->neonBlinkOrientation = GetFlickyNeonBlinkOrOrEven();

	struct Nixi_State clockState;
	clockState.Enter = &ClockFlickyEnter;
	clockState.Update = &ClockFlickyUpdate;
	clockState.Exit = &ClockFlickyExit;
	clockState.Data = (void*) data;
	return clockState;
}

int ClockFlickyEnter(void* d)									// Function that runs at the start of the state of "clock"
{
	struct ClockFlickyData* data = (struct ClockFlickyData*)d;
	data->neonBlinkOrientation = GetFlickyNeonBlinkOrOrEven();				// chose if neon is on or off.
	srand(time(NULL));
	unsigned long long current_time = GetFlickyUnixTime();
	data->nextEffectTime = current_time + minimumTimeToNextEffect + (unsigned long long)(rand() % (maximumTimeToNextEffect - minimumTimeToNextEffect));
	data->effectLength = minimumEffectLength + rand() % (maximumEffectLength - minimumEffectLength);
	return 0;									// exit status condition
}

int ClockFlickyUpdate(void* d)
{
	struct ClockFlickyData* data = (struct ClockFlickyData*)d;

	 unsigned long long current_time = GetFlickyUnixTime();
        if(current_time > data->nextEffectTime)
	{
		printf("Effect!\n");
		srand(time(NULL));
		data->nextEffectTime = current_time + minimumTimeToNextEffect + (unsigned long long)(rand() % (maximumTimeToNextEffect - minimumTimeToNextEffect));

		srand(time(NULL));
		data->effectLength = minimumEffectLength + rand() % (maximumEffectLength - minimumEffectLength);

//		digitalWrite(neons, HIGH);
//		digitalWrite(highVoltagePowerSupply, HIGH);
//		usleep(data->effectLength * 1000);
//             	digitalWrite(highVoltagePowerSupply, LOW);
//		digitalWrite(neons, LOW);


		for(int N = 0; N < 100; N++)
			{
				digitalWrite(highVoltagePowerSupply, HIGH);
				usleep(7 * 1000);
				digitalWrite(highVoltagePowerSupply, LOW);
				usleep(7 * 1000);
				printf("%i", N);
				printf("wee");

		DisplayFlickyTime(
			data->hoursAndMinutesChip, 
			data->secondsAndMicrosecondsAndModeChip, 
			data->use24h, 
			data->neonBlinkOrientation);



	}



	}
	else
	{
		DisplayFlickyTime(

			data->hoursAndMinutesChip, 
			data->secondsAndMicrosecondsAndModeChip, 
			data->use24h, 
			data->neonBlinkOrientation);
	}
	// flicker stuffs
	//if (hour change )
	//{
		// do crazy numbers for  3.2 seconds
	//}

//	srand(time(NULL));	// make random number gen refresh
	return 0;
}

int ClockFlickyExit(void* d)
{
	return 0;
}

//Gay vs straight
int GetFlickyNeonBlinkOrOrEven() {								// chose weathere the neons are on for odd or even numbers of seconds
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

void DisplayFlickyTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation)
{
	//variables for display time
	struct tm *tm_p;
	time_t current_time;
	struct timeval tv;

	current_time = time(NULL);
	tm_p = localtime(&current_time);
	gettimeofday(&tv, NULL);

	char hours = GetFlickyHours(tm_p, use24h);
	char mins = CaculateTime(tm_p->tm_min, minutsBits);
	char sec = CaculateTime(tm_p->tm_sec, secondsBits);
	char decisec = GetFlickyDecisecond(tv);
	int enableNeons = 0;
	if (tm_p->tm_sec % 2 == neonBlinkOrientation && tm_p->tm_hour < 12 )	// only blink during the day. or most of the time
	{
 		enableNeons = HIGH;					// turn neion lights off!
//		enableNeons = LOW;
	}
	else
	{
		enableNeons = LOW;					// turn neion lights on
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

	//Secondd
	if (wiringPiI2CWriteReg8(secondsAndMicrosecondsAndModeChip, chipPortB, (((sec >> 4) & 0x0F) | ((decisec) & 0xF0))) == -1) {

		printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
	}

}

char GetFlickyDecisecond(struct timeval tv) {
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

char GetFlickyHours(struct tm *tm_p, int use24h) {
	int hour = tm_p->tm_hour;
	if (use24h != 1) {
		if (hour > 12) {
			hour = hour - 12;
		}
	}
	return CaculateTime(hour, hoursBits);
}

long long unsigned GetFlickyUnixTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long millisecondsSinceEpoch = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	return millisecondsSinceEpoch;
}
