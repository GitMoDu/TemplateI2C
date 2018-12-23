// I2CDriverTemplate.h

#if !defined(_I2CDRIVERTEMPLATE_h)
#define _I2CDRIVERTEMPLATE_h

#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>

#define I2C_DRIVER_TEMPLATE_MIN_MICROS_BETWEEN_MESSAGE 100

template <typename WireClass, const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN>
class I2CDriverTemplate
{
private:
	uint8_t DeviceAddress = 0;
	WireClass* I2CInstance = nullptr;

protected:
	TemplateMessageI2C<MessageMaxSize> Message;

protected:
	inline bool WriteCurrentMessage()
	{
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write((uint8_t *)Message.GetRaw(), Message.GetLength());

		return I2CInstance->endTransmission() == 0;
	}

	inline bool SendMessageHeader(const uint8_t header)
	{
		Message.Clear();
		Message.SetHeader(header);

		return WriteCurrentMessage();
	}

	inline bool SendMessageDual16(const uint8_t header, uint16_t value1, uint16_t value2)
	{
		Message.Clear();
		Message.SetHeader(header);
		Message.Set16BitPayload(0, value1);
		Message.Set16BitPayload(2, value2);

		return WriteCurrentMessage();
	}

	inline bool SendMessageSingle16(const uint8_t header, uint16_t value)
	{
		Message.Clear();
		Message.SetHeader(header);
		Message.Set16BitPayload(0, value);

		return WriteCurrentMessage();
	}

	inline bool SendMessageSingle32(const uint8_t header, uint32_t value)
	{
		Message.Clear();
		Message.SetHeader(header);
		Message.Set32BitPayload(value);

		return WriteCurrentMessage();
	}

protected:
	virtual uint32_t GetDeviceId() { return 0; }

public:
	I2CDriverTemplate()
	{
	}

	bool ValidateIC()
	{
#ifndef MOCK_DRIVER
		if (!SendMessageHeader(I2C_SLAVE_BASE_HEADER_DEVICE_ID))
		{
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.println(F("Device not found."));
#endif
			return false;
		}

		Message.Clear();
		delayMicroseconds(I2C_DRIVER_TEMPLATE_MIN_MICROS_BETWEEN_MESSAGE);
		I2CInstance->requestFrom((uint8_t)DeviceAddress, (uint8_t)5);

		while (I2CInstance->available())
		{
			if (!Message.Write(I2CInstance->read()))
			{
				break;
			}
		}

		if (Message.GetLength() != 5 ||
			Message.GetHeader() != I2C_SLAVE_BASE_HEADER_DEVICE_ID)
		{
			//Invalid return message.
#ifdef DEBUG_I2C_DRIVER_TEMPLATE
			Serial.print(F("Invalid return message. Size: "));
			Serial.print(Message.GetLength());
			Serial.print(F(" Header: "));
			Serial.println(Message.GetHeader());
#endif
			return false;
		}

		if (Message.Get32BitPayload(0) != GetDeviceId())
		{
			//Invalid expected version code.
			return false;
		}
#endif

		delayMicroseconds(I2C_DRIVER_TEMPLATE_MIN_MICROS_BETWEEN_MESSAGE);

		return true;
	}

	virtual bool Setup(WireClass* i2CInstance, const uint8_t deviceAddress)
	{
		DeviceAddress = deviceAddress;

		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
			//Invalid I2C Address
			return false;
		}

		I2CInstance = i2CInstance;
		if (I2CInstance == nullptr)
		{
			return false;
		}

		for (uint8_t i = 0; i < 2; i++)
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
				delay(50);
			}
		}
		return false;
	}
};
#endif

