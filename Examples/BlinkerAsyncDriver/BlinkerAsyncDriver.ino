/* Blinker Template I2C Async Driver Example
*
* Depends on Task Scheduler (https://github.com/arkhipenko/TaskScheduler).
* Calls BlinkerApi over I2C without blocking delays.
*
* Implements Blinker example Template I2C Async Driver.
* Includes Serial interface to test API.
*
*
*/

#define DEBUG_TEMPLATE_I2C

// Tracks when an I2C transaction starts and ends.
#define DEBUG_PIN 5

#define SERIAL_BAUD_RATE 115200


#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TScheduler.hpp>

#include "BlinkerI2cAsyncDriver.h"


TS::Scheduler SchedulerBase{};

BlinkerI2cAsyncDriver Driver(SchedulerBase, Wire);

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;
	Serial.println(F("Template I2C Blinker Async Driver"));
	Serial.println();
	Serial.println(F("Serial Tester:"));
	Serial.println(F("\t0 - Led Toggle"));
	Serial.println(F("\t1 - Led Off"));
	Serial.println(F("\t2 - Led On"));
	Serial.println(F("\ti - Get Device Id"));
	Serial.println(F("\tr - Reset Device"));
	Serial.println(F("\tl - Long Request"));
	Serial.println();

	Wire.begin();

	// Even an 8MHz AVR can clock in I2C @ 400KHz.
	Wire.setClock((uint32_t)400000);
}

void loop()
{
	if (Serial.available())
	{
		const int8_t in = Serial.read();

		uint32_t helper = 0;

		switch (in)
		{
		case '0':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			Driver.LedToggle();
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, LOW);
#endif
			Serial.println(F("LED toggled"));
			break;
		case '1':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			Driver.LedOff();
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, LOW);
#endif
			Serial.println(F("LED off"));
			break;
		case '2':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			Driver.LedOn();
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, LOW);
#endif
			Serial.println(F("LED on"));
			break;
		case 'i':
		case 'I':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			if (Driver.GetDeviceId(helper))
			{
#if defined(DEBUG_PIN)
				digitalWrite(DEBUG_PIN, LOW);
#endif
				Serial.print(F("Device ID:\t"));
				Serial.println(helper);
			}
			else
			{
#if defined(DEBUG_PIN)
				digitalWrite(DEBUG_PIN, LOW);
#endif
				Serial.println(F("Failed to read device ID."));
			}
			break;
		case 'r':
		case 'R':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			if (Driver.ResetDevice())
			{
#if defined(DEBUG_PIN)
				digitalWrite(DEBUG_PIN, LOW);
#endif
				Serial.println(F("Device Reset"));
			}
			else
			{
#if defined(DEBUG_PIN)
				digitalWrite(DEBUG_PIN, LOW);
#endif
				Serial.println(F("Failed to reset device."));
			}
			break;
		case 'l':
		case 'L':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			helper = Driver.LongRequest();
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, LOW);
#endif
			Serial.print(F("Long Request got: "));
			Serial.println(helper);
			break;
		case 'a':
		case 'A':
#if defined(DEBUG_PIN)
			digitalWrite(DEBUG_PIN, HIGH);
#endif
			if (Driver.LongRequestAsync(OnLongRequestResult))
			{
				// Wait for reply.
			}
			else
			{
#if defined(DEBUG_PIN)
				digitalWrite(DEBUG_PIN, LOW);
#endif
				Serial.println(F("Long Request async failed."));
			}
			break;
		default:
			break;
		}
	}

	SchedulerBase.execute();
}

void OnLongRequestResult(const uint8_t result)
{
#if defined(DEBUG_PIN)
	digitalWrite(DEBUG_PIN, LOW);
#endif
	Serial.print(F("Long Request Async got: "));
	Serial.println(result);
}
