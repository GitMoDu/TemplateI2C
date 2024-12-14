/* Blinker Template I2C Driver Example
*
* Calls BlinkerApi over I2C.
*
* Implements Blinker example Template I2C Driver.
* Includes Serial interface to test API.
* 
*
*/



#define DEBUG_TEMPLATE_I2C

#define SERIAL_BAUD_RATE 115200

#include "BlinkerI2cDriver.h"

BlinkerI2cDriver Driver(Wire);


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;
	Serial.println(F("Template I2C Blinker Driver"));
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

		uint32_t deviceId = 0;

		if (Driver.CheckDevice())
		{
			switch (in)
			{
			case '0':
				Driver.LedToggle();
				Serial.println(F("LED toggled"));
				break;
			case '1':
				Driver.LedOff();
				Serial.println(F("LED off"));
				break;
			case '2':
				Driver.LedOn();
				Serial.println(F("LED on"));
				break;
			case 'i':
			case 'I':
				if (Driver.GetDeviceId(deviceId))
				{
					Serial.print(F("Device ID: "));
					Serial.println(deviceId);
				}
				else
				{
					Serial.println(F("Failed to read device ID."));
				}
				break;
			case 'r':
			case 'R':
				if (Driver.ResetDevice())
				{
					Serial.println(F("Device Reset"));
				}
				else
				{
					Serial.println(F("Failed to reset device."));
				}
				break;
			case 'l':
			case 'L':
				Serial.print(F("Long Request got: "));
				Serial.println(Driver.LongRequest());
				break;
			default:
				break;
			}
		}
		else
		{
			Serial.println(F("Device not responding"));
		}
	}
}
