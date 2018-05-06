#ifndef CLOCK_H
#define CLOCK_H

char CaculateTime(int number, char map[2][10]);
void DisplayTime(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int use24h, int neonBlinkOrientation);
char GetHours(struct tm *tm_p, int use24h);
char GetDecisecond(struct timeval tv);
void DisplayNumber(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number);
int GetNeonBlinkOrientation();

#endif 
