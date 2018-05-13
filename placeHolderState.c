#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "main.h"
#include "placeHolderState.h"


struct PlaceHolderData {
	int hoursAndMinutesChip;
	int secondsAndMicrosecondsAndModeChip;
	int numberToDisplay;
};

struct Nixi_State CreatePlaceHolderState(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int numberToDisplay)
{
	struct PlaceHolderData *data = malloc(sizeof(struct PlaceHolderData));
	data->hoursAndMinutesChip = hoursAndMinutesChip;
	data->secondsAndMicrosecondsAndModeChip = secondsAndMicrosecondsAndModeChip;
	data->numberToDisplay = numberToDisplay;

	struct Nixi_State placeHolderState;
	placeHolderState.Enter = &PlaceHolderEnter;
	placeHolderState.Update = &PlaceHolderUpdate;
	placeHolderState.Exit = &PlaceHolderExit;
	placeHolderState.Data = (void*)data;
	return placeHolderState;
}

int PlaceHolderEnter(void* d)
{
    struct PlaceHolderData* data = (struct PlaceHolderData*)d;
    DisplayNumber(
        data->hoursAndMinutesChip, 
        data->secondsAndMicrosecondsAndModeChip, 
        data->numberToDisplay);
        return 0;
}

int PlaceHolderUpdate(void* d)
{
	return 0;
}

int PlaceHolderExit(void* d)
{
	return 0;
}