#define DEBUG_LOG
#define WAIT_FOR_LOGGER
#define DEBUG_I2C

#define _TASK_OO_CALLBACKS

#define SERIAL_BAUD_RATE 115200
#define DEBUG_TESTS



#include <Arduino.h>
#include <TaskScheduler.h>
#include <Wire.h>

#include <I2CAsyncDriverTemplate.h>
#include "ExampleSlaveApi.h"


///Process scheduler.
Scheduler SchedulerBase;
///

///I2C ExampleDriverTask
class ExampleI2CAsyncDriver : public I2CAsyncDriverTemplate<TwoWire, ExampleApi::DeviceAddress>
{
public:
	ExampleI2CAsyncDriver(Scheduler* scheduler) : I2CAsyncDriverTemplate<TwoWire, ExampleApi::DeviceAddress>(scheduler)
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
	bool OnSetup() { return true; }
};

ExampleI2CAsyncDriver ExampleAsyncDriver(&SchedulerBase);
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
	Serial.println(F("Arduino I2C ExampleDriverTask Slave Tester"));
#endif

#ifndef MOCK_DRIVER
	Wire.setClock((uint32_t)400000);
	Wire.begin();
#endif

	if (!ExampleAsyncDriver.Setup(&Wire))
	{
#ifdef DEBUG_LOG
		Serial.println(F("ExampleDriverTask Setup Failed."));
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
	SchedulerBase.execute();
}

#ifdef DEBUG_TESTS
void InjectTestMessages()
{
	Serial.println(F("Injecting test messages."));

	delay(1);
	ExampleAsyncDriver.Start();
	ExampleAsyncDriver.Stop();
}
#endif
