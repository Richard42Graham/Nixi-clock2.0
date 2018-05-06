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

void Initialize();

#endif /* MAIN_H */
