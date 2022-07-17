



#define SERIAL_BAUD_RATE 115200

#define DEBUG_TEMPLATE_I2C

#include "I2CExampleApiInclude.h"
#include <TemplateI2CSlave.h>


// Example controller.
inline void SetupCallbacks();
class ExampleControllerClass
{
public:
	ExampleControllerClass()
	{
		pinMode(LED_BUILTIN, OUTPUT);
	}

	void Start()
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}

	void Stop()
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}
};

// ExampleSlave.
class ExampleSlaveClass : public TemplateI2CSlave<ExampleApi::DeviceAddress, ExampleApi::DeviceId>
{
private:
	ExampleControllerClass* Controller = nullptr;
	using TemplateI2CSlave<ExampleApi::DeviceAddress, ExampleApi::DeviceId>::IncomingProcessingMessage;

public:
	ExampleSlaveClass(ExampleControllerClass* controller) : TemplateI2CSlave<ExampleApi::DeviceAddress, ExampleApi::DeviceId>()
	{
		Controller = controller;
	}

protected:
	virtual bool ProcessMessage()
	{
		switch (IncomingProcessingMessage.GetHeader())
		{
		case ExampleApi::Start::Header:
			if (IncomingProcessingMessage.Length == ExampleApi::Start::CommandLength)
			{
				Controller->Start();
			}
			break;
		case ExampleApi::Stop::Header:
			if (IncomingProcessingMessage.Length == ExampleApi::Stop::CommandLength)
			{
				Controller->Stop();
				return true;
			}
			break;
		default:
			break;
		}

		return false;
	}

	virtual bool SetupWireCallbacks()
	{
		SetupCallbacks();

		return true;
	}
};

//
ExampleControllerClass Controller;

ExampleSlaveClass ExampleSlave(&Controller);
//

void ReceiveEvent(int16_t length)
{
	ExampleSlave.OnReceive(length);
}

void RequestEvent()
{
	ExampleSlave.OnRequest();
}

void SetupCallbacks()
{
	Wire.onReceive(ReceiveEvent);
	Wire.onRequest(RequestEvent);
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
}

#ifdef DEBUG_TESTS
void InjectTestMessages()
{
	Serial.println(F("Injecting test messages."));
}
#endif

