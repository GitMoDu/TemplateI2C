// TemplateI2CDriver.h

#ifndef _TEMPLATE_I2C_DRIVER_h
#define _TEMPLATE_I2C_DRIVER_h

#include <Arduino.h>
#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>
#include <Wire.h>

template <const uint8_t DeviceAddress,
	const uint32_t DeviceId,
	const uint32_t MillisBeforeResponse = 1>
	class TemplateI2CDriver
{
private:
	static const uint8_t SetupRetryMaxCount = 3;
	TwoWire* I2CInstance;

protected:
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> OutgoingMessage;
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> IncomingMessage;

public:
	const uint32_t GetDeviceId() { return DeviceId; }

public:
	TemplateI2CDriver(TwoWire* i2cInstance)
		: I2CInstance(i2cInstance)
	{
	}

	virtual const bool Setup()
	{
		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
			Serial.println(F("Invalid I2C Address."));
#endif
			return false;
		}

		if (I2CInstance != nullptr)
		{
			return true;
		}

#ifdef DEBUG_TEMPLATE_I2C_DRIVER
		Serial.println(F("Device Not detected."));
#endif

		return false;
	}

	virtual const bool CheckDevice()
	{
#ifdef I2C_DRIVER_MOCK_I2C
		return true;
#else
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		return SendMessageHeader(0) &&
			GetResponse(BaseAPI::GetDeviceId.ResponseLength) &&
			IncomingMessage.Get32Bit(0) == DeviceId;
#else
		return SendMessageHeader(0); // Just check for send Ack on the device address.
#endif
#endif
	}

protected:
	const bool WriteCurrentMessage()
	{
#ifndef I2C_DRIVER_MOCK_I2C
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write(OutgoingMessage.Data, OutgoingMessage.Length);

		return I2CInstance->endTransmission() == 0;
#else
		return true;
#endif		
	}

	const bool GetResponse(const uint8_t requestSize)
	{
#ifdef I2C_DRIVER_MOCK_I2C
		return true;
#else
		IncomingMessage.Clear();
		if (I2CInstance->requestFrom(DeviceAddress, requestSize))
		{
			while (I2CInstance->available())
			{
				IncomingMessage.FastWrite(I2CInstance->read());
			}

			return IncomingMessage.Length == requestSize;
		}

		return false;
#endif		
	}

	// Quick message sender.
	const bool SendMessageHeader(const uint8_t header)
	{
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Length = 1;

		return WriteCurrentMessage();
	}
};
#endif