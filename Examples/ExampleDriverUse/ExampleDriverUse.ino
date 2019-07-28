
#define DEBUG_LOG
#define WAIT_FOR_LOGGER
#define DEBUG_I2C

#define DEBUG_TESTS

//#define MOCK_I2C_DRIVER

#define SERIAL_BAUD_RATE 115200

#include "ExampleSlaveApi.h"
#include <I2CDriverTemplate.h>
#include <Wire.h>

///I2C ExampleDriver
class ExampleI2CDriver : public I2CDriverTemplate<TwoWire, ExampleApi::DeviceAddress>
{
public:
	ExampleI2CDriver() : I2CDriverTemplate<TwoWire, ExampleApi::DeviceAddress>()
	{
	}

public:
	uint32_t GetDeviceId() { return ExampleApi::DeviceId; }


	void Start()
	{
		SendMessageHeader(ExampleApi::RequestHeader::Start);
	}

	void Stop()
	{
		SendMessageHeader(ExampleApi::RequestHeader::Stop);
	}

protected:
	virtual bool OnSetup() { return true; }
};


ExampleI2CDriver ExampleDriver;
///

void Halt()
{
#ifdef DEBUG_LOG
	Serial.println("Critical Error");
	delay(1000);
#endif	
	while (1);;
}

void setup()
{
#ifdef DEBUG_LOG
	Serial.begin(SERIAL_BAUD_RATE);
#ifdef WAIT_FOR_LOGGER
	while (!Serial)
		;
#endif
	delay(1000);
	Serial.println(F("Arduino I2C ExampleDriver Slave Tester"));
#endif

#ifndef MOCK_I2C_DRIVER
	Wire.setClock((uint32_t)400000);
	Wire.begin();
#endif

	if (!ExampleDriver.Setup(&Wire))
	{
#ifdef DEBUG_LOG
		Serial.println(F("ExampleDriver Setup Failed."));
#endif
		Halt();
	}

#ifdef DEBUG_LOG
	Serial.println(F("Start!"));
#endif


#ifdef DEBUG_TESTS
	InjectTestMessages();
#endif

}

void loop()
{
}

#ifdef DEBUG_TESTS

void InjectTestMessages()
{
	Serial.println(F("Injecting test messages."));

	delay(1);
	ExampleDriver.Start();
	delay(10000);
	ExampleDriver.Stop();
}
#endif

