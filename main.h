#ifndef MAIN_H
#define MAIN_H

extern char hoursBits[2][10];
extern char minutsBits[2][10];
extern char secondsBits[2][10];
extern char deciseconds[2][10];

// commonly used pins, labled so its easyer to read my code.

//The on/off switch
//By default it should be pulled up high
extern const int onOffSwitch;
extern const int amLed;
extern const int pmLed;
extern const int neons;
extern const int highVoltagePowerSupply;

// i2c port expander usefull adresses.
extern const int hoursAndMinutesChipAddress;
extern const int secondsAndMicrosecondsAndModeChipAddress;
extern const int chipPortA;
extern const int chipPortB;


//State struct
struct Nixi_State {
	//Enter:  Is called when entering the state
	int (*Enter)();

	//Update: is called ever "frame"
	int (*Update)();

	//Exit: Is called just we exit the state, and just before Enter is called on the next state 
	int (*Exit)();

	void* Data;
};

void Initialize();
char CaculateTime(int number, char map[2][10]);
void DisplayNumber(int hoursAndMinutesChip, int secondsAndMicrosecondsAndModeChip, int number);

#endif /* MAIN_H */

