
#define DEBUG_LOG
#define WAIT_FOR_LOGGER
#define DEBUG_TEMPLATE_I2C_DRIVER

#define DEBUG_TESTS

//#define I2C_DRIVER_MOCK_I2C

#define SERIAL_BAUD_RATE 115200

#define _TASK_OO_CALLBACKS
#include <TaskScheduler.h>

#include "I2CExampleApiInclude.h"
#include <TemplateAsyncI2CDriver.h>

HandleI2C I2CHandle(Wire);

// Process scheduler.
Scheduler SchedulerBase;

///I2C ExampleDriver
class ExampleI2CDriver : public TemplateAsyncI2CDriver<ExampleApi::DeviceAddress, ExampleApi::DeviceId>
{
public:
	ExampleI2CDriver(const Scheduler* scheduler, HandleI2C& handle) 
		: TemplateAsyncI2CDriver<ExampleApi::DeviceAddress, ExampleApi::DeviceId>(scheduler, handle)
	{
	}

public:
	void Start()
	{
		SendMessageHeader(ExampleApi::Start.Header);
	}

	void Stop()
	{
		SendMessageHeader(ExampleApi::Stop.Header);
	}

protected:
	virtual bool OnSetup() { return true; }

};


ExampleI2CDriver ExampleDriver(&SchedulerBase, I2CHandle);
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
	Serial.println(F("Arduino TemplateDriver Slave Tester"));
#endif

#ifndef MOCK_I2C_DRIVER
	Wire.setClock((uint32_t)400000);
	Wire.begin();
#endif

	if (!ExampleDriver.Setup())
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
	SchedulerBase.execute();
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

