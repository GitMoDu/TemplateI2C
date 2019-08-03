
//#define DEBUG_LOG
//#define WAIT_FOR_LOGGER
//#define DEBUG_I2C

//#define DEBUG_TESTS


#define SERIAL_BAUD_RATE 115200

#include "ExampleSlaveApi.h"
#include <AbstractI2CSlave.h>

#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
#define _TASK_OO_CALLBACKS
#include <TaskScheduler.h>
///Process scheduler.
Scheduler SchedulerBase;
///
#endif

#ifdef ARDUINO_ARCH_AVR
#define BLINK_PIN 13
#else
#error Define a pin
#endif // ARDUINO_ARCH_AVR


///Example controller.
class ExampleControllerClass
{
public:
	void Start()
	{
		pinMode(BLINK_PIN, OUTPUT);
		digitalWrite(BLINK_PIN, true);
	}

	void Stop()
	{
		pinMode(BLINK_PIN, INPUT);
	}
};
///
///I2C ExampleSlave
class ExampleSlaveClass : public AbstractI2CSlaveTask<ExampleApi::DeviceAddress>
{
private:
	ExampleControllerClass* Controller = nullptr;

public:
#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	ExampleSlaveClass(Scheduler* scheduler, ExampleControllerClass* controller) : AbstractI2CSlaveTask<ExampleApi::DeviceAddress>(scheduler)
#else
	ExampleSlaveClass(ExampleControllerClass* controller) : AbstractI2CSlaveTask<ExampleApi::DeviceAddress>()
#endif	
	{
		Controller = controller;
	}

protected:
	bool ProcessMessage(IMessageI2C* currentMessage)
	{
		switch (currentMessage->GetHeader())
		{
		case ExampleApi::Start:
			if (currentMessage->GetLength() == I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				Controller->Start();
			}
			break;
		case ExampleApi::Stop:
			if (currentMessage->GetLength() == I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				Controller->Stop();
			}
			break;
		default:
			break;
		}
		return false;
	}

	bool OnSetup() { return Controller != nullptr; }
};


ExampleControllerClass Controller;
#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
ExampleSlaveClass ExampleSlave(&SchedulerBase, &Controller);
#else
ExampleSlaveClass ExampleSlave(&Controller);
#endif
///


void setup()
{
#ifdef DEBUG_LOG
	Serial.begin(SERIAL_BAUD_RATE);
#ifdef WAIT_FOR_LOGGER
	while (!Serial)
		;
#endif
	delay(1000);
	Serial.println(F("Example I2C Slave Device"));
#endif

	if (!ExampleSlave.Setup())
	{
#ifdef DEBUG_LOG
		Serial.println(F("Setup Failed."));
#endif
		while (1);;
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
	ExampleSlave.Callback();
}

#ifdef DEBUG_TESTS
void InjectTestMessages()
{
	Serial.println(F("Injecting test messages."));

}
#endif

