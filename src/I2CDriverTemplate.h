// I2CDriverTemplate.h

#ifndef _I2CDRIVERTEMPLATE_h
#define _I2CDRIVERTEMPLATE_h

#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>
#include <I2CDriverCommon.h>

template <typename WireClass,
	const uint8_t DeviceAddress,
	const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN,
	const uint32_t ReadAfterWriteDelayMicros = 20>
	class I2CDriverTemplate : public I2CDriverCommon
{
private:
	static const uint8_t SetupRetryMaxCount = 3;
	WireClass* I2CInstance = nullptr;

	const uint8_t DEVICE_ID_HEADER = I2C_SLAVE_BASE_HEADER_DEVICE_ID;
	const uint8_t DEVICE_ID_RESPONSE_SIZE = I2C_MESSAGE_LENGTH_32BIT_X1;

protected:
	TemplateMessageI2C<MessageMaxSize> OutgoingMessage;
	TemplateMessageI2C<MessageMaxSize> IncomingMessage;

public:
	virtual uint32_t GetDeviceId() { return 0; }

protected:
	virtual bool OnSetup() { return true; }

public:
	I2CDriverTemplate(WireClass* i2cInstance): I2CDriverCommon()
	{
		I2CInstance = i2cInstance;
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
			if (CheckDevice())
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

	virtual bool CheckDevice()
	{
		SendMessageHeader(DEVICE_ID_HEADER);
		
		delayMicroseconds(ReadAfterWriteDelayMicros);

		if (GetResponse(DEVICE_ID_RESPONSE_SIZE)
			&& (IncomingMessage.GetHeader() == DEVICE_ID_HEADER)
			&& (IncomingMessage.Get32BitPayload(0) == GetDeviceId()))
		{
			return true;
		}

		return false;
	}

	bool GetResponse(const uint8_t requestSize)
	{
		IncomingMessage.Clear();
		
		I2CInstance->requestFrom(DeviceAddress, requestSize);

		while (I2CInstance->available())
		{
			IncomingMessage.FastWrite(I2CInstance->read());
		}

		return IncomingMessage.GetLength() == requestSize;
	}

protected:
	inline bool WriteCurrentMessage()
	{
#ifndef MOCK_I2C_DRIVER
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write(OutgoingMessage.GetRaw(), OutgoingMessage.GetLength());
		
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

