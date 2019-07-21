#define DEBUG_LOG
#define WAIT_FOR_LOGGER
#define DEBUG_I2C

#define _TASK_OO_CALLBACKS
#define I2C_DRIVER_ASYNC_ENABLE

#define SERIAL_BAUD_RATE 115200
#define DEBUG_TESTS



#include <Arduino.h>
#include <TaskScheduler.h>

#include <I2CDriverTemplateTask.h>
#include "ExampleSlaveApi.h"

#include <Wire.h>
#if defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) ||defined(__AVR_ATmega328P__)
#define WireType TwoWire
#else
#define WireType Wire
#endif


///Process scheduler.
Scheduler SchedulerBase;
///

///I2C ExampleDriverTask
//template<typename WireClass>
//class ExampleI2CDriverTask : public I2CDriverTemplateTask<WireClass, ExampleApi::DeviceAddress>
//{
//
//public:
//	ExampleI2CDriverTask(Scheduler* scheduler) : I2CDriverTemplateTask<WireClass, ExampleApi::DeviceAddress>()
//	{
//
//	}
//
//	bool OnSetup()
//	{
//		return true;
//	}
//
//	void Start()
//	{
//		SendMessageHeader(ExampleApi::RequestHeader::Start);
//	}
//
//	void Stop()
//	{
//		SendMessageHeader(ExampleApi::RequestHeader::Stop);
//	}
//};

//ExampleI2CDriverTask<WireType> ExampleDriver(&SchedulerBase);
I2CDriverTemplateTask<WireType, ExampleApi::DeviceAddress> ExampleDriver(&SchedulerBase);
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

	if (!ExampleDriver.Setup(&Wire))
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
	//ExampleDriver.Start();
	delay(10000);
	//ExampleDriver.Stop();
}
#endif

