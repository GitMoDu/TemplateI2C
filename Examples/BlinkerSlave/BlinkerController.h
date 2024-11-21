// BlinkerController.h

#ifndef _BLINKERCONTROLLER_h
#define _BLINKERCONTROLLER_h

#include <Arduino.h>

class BlinkerController
{
private:
	volatile uint8_t LongValue = 1;

public:
	BlinkerController()
	{
		pinMode(LED_BUILTIN, OUTPUT);
	}

	void WriteLed(const bool value)
	{
		digitalWrite(LED_BUILTIN, value);
	}

	void ToggleLed()
	{
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	}
};


#endif

