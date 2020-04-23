// I2CAsyncDriverTemplate.h
#ifndef _I2C_ASYNC_DRIVER_TEMPLATE_h
#define _I2C_ASYNC_DRIVER_TEMPLATE_h



#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include <RingBufCPP.h>

#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>



template<typename WireClass,
	const uint8_t DeviceAddress,
	const uint32_t MinDelayMicros = 100,
	const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN,
	const uint8_t OutQueueSize = 10>
	class I2CAsyncDriverTemplate : Task
{
private:
	const uint32_t MinDelayMillis = (MinDelayMicros / 1000) + ((MinDelayMicros % 1000) > 0);

	const uint8_t SetupRetryMaxCount = 3;

	WireClass* I2CInstance = nullptr;

protected:
	TemplateMessageI2C<MessageMaxSize> OutgoingMessage;
	TemplateMessageI2C<MessageMaxSize> IncomingMessage;


	RingBufCPP<TemplateMessageI2C<MessageMaxSize>, OutQueueSize> OutMessageQueue;

	uint32_t LastSentMicros = 0;

protected:
	virtual bool OnSetup() { return true; }

public:
	virtual uint32_t GetDeviceId() { return 0; }

public:
	I2CAsyncDriverTemplate(Scheduler* scheduler, WireClass* i2cInstance) : Task(MinDelayMillis, TASK_FOREVER, scheduler, false)
	{
		I2CInstance = i2cInstance;
	}

	bool OnEnable()
	{
		return true;
	}

	void OnDisable()
	{
	}

	bool Callback()
	{
		if (OutMessageQueue.isEmpty())
		{
			disable();
		}
		else if (CanSendNow())
		{
			if (OutMessageQueue.pull(OutgoingMessage))
			{
				WriteCurrentMessageNow();
			}

			return true;
		}

		return false;
	}

	bool Setup()
	{
		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.println(F("Invalid I2C Address."));
#endif
			return false;
		}

		if (I2CInstance == nullptr || !OnSetup())
		{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.println(F("Device Not Setup."));
#endif
			return false;
		}

		for (uint8_t i = 0; i < SetupRetryMaxCount; i++)
		{
			if (ValidateIC())
			{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
				Serial.println(F("Device detected."));
#endif
				return true;
			}
			else
			{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
				Serial.println(F("Device Not detected."));
#endif
			}
		}
		return false;
	}

protected:
	//Only returns true if message was written.
	inline bool WriteCurrentMessageNow()
	{
		LastSentMicros = micros();

#ifndef MOCK_I2C_DRIVER
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write(OutgoingMessage.GetRaw(), OutgoingMessage.GetLength());

		return I2CInstance->endTransmission() == 0;
#else
		return true;
#endif		
	}

	inline bool GetResponseNow(const uint8_t requestSize)
	{
		I2CInstance->requestFrom(DeviceAddress, requestSize);

		IncomingMessage.Clear();
		while (I2CInstance->available())
		{
			IncomingMessage.FastWrite(I2CInstance->read());
		}

		return IncomingMessage.GetLength() == requestSize;
	}

	inline bool CanSendNow()
	{
		return (micros() - LastSentMicros) >= MinDelayMicros;
	}

	inline bool WriteCurrentMessage()
	{
		if (CanSendNow())
		{
			return WriteCurrentMessageNow();
		}
		else
		{
			OutMessageQueue.addForce(OutgoingMessage);
			enableIfNot();
		}

		return false;
	}

	bool ValidateIC()
	{
#ifndef MOCK_I2C_DRIVER
		if (!SendMessageHeader(I2C_SLAVE_BASE_HEADER_DEVICE_ID))
		{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.println(F("Device not found."));
#endif
			return false;
		}

		IncomingMessage.Clear();
		delayMicroseconds(MinDelayMicros);

		if (!GetResponseNow(I2C_MESSAGE_LENGTH_32BIT_X1))
		{
			return false;
		}

		if (IncomingMessage.GetLength() != I2C_MESSAGE_LENGTH_32BIT_X1 ||
			IncomingMessage.GetHeader() != I2C_SLAVE_BASE_HEADER_DEVICE_ID)
		{
			//Invalid return message.
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.print(F("Invalid return message. Size: "));
			Serial.print(IncomingMessage.GetLength());
			Serial.print(F(" Header: "));
			Serial.println(IncomingMessage.GetHeader());
#endif
			return false;
		}

		if (IncomingMessage.Get32BitPayload(0) != GetDeviceId())
		{
			//Invalid expected version code.
			return false;
		}
#endif

		return true;
	}

	//TODO: 
	/*bool GetResponseAsync()
	{

	}*/
	bool SendMessageHeader(const uint8_t header)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);

		return WriteCurrentMessage();
	}

	bool SendMessageDual16(const uint8_t header, uint16_t value1, uint16_t value2)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Append16BitPayload(value1);
		OutgoingMessage.Append16BitPayload(value2);

		return WriteCurrentMessage();
	}

	bool SendMessageSingle16(const uint8_t header, uint16_t value)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Append16BitPayload(value);

		return WriteCurrentMessage();
	}

	bool SendMessageSingle32(const uint8_t header, uint32_t value)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Append32BitPayload(value);

		return WriteCurrentMessage();
	}
};
#endif