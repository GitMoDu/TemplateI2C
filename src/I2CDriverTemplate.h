// I2CDriverTemplate.h

#ifndef _I2CDRIVERTEMPLATE_h
#define _I2CDRIVERTEMPLATE_h

#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>

template <typename WireClass, 
	const uint8_t DeviceAddress, 
	const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN>
class I2CDriverTemplate
{
private:
	static const uint8_t SetupRetryMaxCount = 3;
	WireClass* I2CInstance = nullptr;

protected:
	TemplateMessageI2C<MessageMaxSize> OutgoingMessage;
	TemplateMessageI2C<MessageMaxSize> IncomingMessage;

public:
	virtual uint32_t GetDeviceId() { return 0; }

protected:
	virtual bool OnSetup() { return true; }

public:
	I2CDriverTemplate()
	{
	}

	bool GetResponse(const uint8_t requestSize)
	{
		I2CInstance->requestFrom(DeviceAddress, requestSize);

		while (I2CInstance->available())
		{
			if (!IncomingMessage.Append(I2CInstance->read()))
			{
				break;
			}
		}

		return IncomingMessage.GetLength() == requestSize;
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
		delayMicroseconds(500);
		I2CInstance->requestFrom(DeviceAddress, (uint8_t)I2C_MESSAGE_LENGTH_32BIT_X1);

		while (I2CInstance->available())
		{
			if (!IncomingMessage.Append(I2CInstance->read()))
			{
				break;
			}
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

	bool Setup(WireClass* i2CInstance)
	{
		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.println(F("Invalid I2C Address."));
#endif
			return false;
		}

		I2CInstance = i2CInstance;
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
	inline bool WriteCurrentMessage()
	{
#ifndef MOCK_I2C_DRIVER
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write((uint8_t*)OutgoingMessage.GetRaw(), OutgoingMessage.GetLength());

		return I2CInstance->endTransmission() == 0;
#else
		return true;
#endif		
	}

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

