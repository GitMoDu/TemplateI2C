// TemplateI2CDriver.h

#ifndef _TEMPLATE_I2C_DRIVER_h
#define _TEMPLATE_I2C_DRIVER_h

#include <Arduino.h>
#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>
#include <Wire.h>

template <const uint8_t DeviceAddress,
	const uint32_t DeviceId,
	const uint32_t MillisBeforeResponse = 20>
	class TemplateI2CDriver
{
private:
	static const uint8_t SetupRetryMaxCount = 3;
	TwoWire* I2CInstance = nullptr;

protected:
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> OutgoingMessage;
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> IncomingMessage;

public:
	const uint32_t GetDeviceId() { return DeviceId; }

public:
	TemplateI2CDriver(TwoWire* i2cInstance)
	{
		I2CInstance = i2cInstance;
	}

	virtual bool Setup()
	{
		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
			Serial.println(F("Invalid I2C Address."));
#endif
			return false;
		}

		if (I2CInstance == nullptr)
		{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
			Serial.println(F("Device Not Setup."));
#endif
			return false;
		}

		for (uint8_t i = 0; i < SetupRetryMaxCount; i++)
		{
			if (CheckDevice())
			{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
				Serial.println(F("Device detected."));
#endif
				return true;
			}
			else
			{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
				Serial.println(F("Device Not detected."));
#endif
			}
		}
		return false;
	}

	bool RequestDeviceIdAsync()
	{
		return SendMessageHeader(BaseAPI::GetDeviceId.Header);
	}

	bool GetDeviceIdAsync(uint32_t& deviceIdFromRead)
	{
		if (GetResponse(BaseAPI::GetDeviceId.ResponseLength))
		{
			deviceIdFromRead = IncomingMessage.Get32Bit(0);
			return true;
		}

		return false;
	}

	virtual bool CheckDevice()
	{
#ifdef I2C_DRIVER_MOCK_I2C
		return true;
#else

		bool Ok = SendMessageHeader(BaseAPI::GetDeviceId.Header);

#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		delay(MillisBeforeResponse);

		if (Ok)
		{
			if (!GetResponse(BaseAPI::GetDeviceId.ResponseLength)
				|| (IncomingMessage.Get32Bit(0) != GetDeviceId()))
			{
				Ok = false;
			}
		}
#else

		return Ok;
#endif
#endif
	}

	bool GetResponse(const uint8_t requestSize)
	{
		IncomingMessage.Clear();

#ifdef I2C_DRIVER_MOCK_I2C
		return true;
#else
		if (I2CInstance->requestFrom(DeviceAddress, requestSize, true))
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

protected:
	bool WriteCurrentMessage()
	{
#ifndef I2C_DRIVER_MOCK_I2C
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write((uint8*)OutgoingMessage.Data, OutgoingMessage.Length);

		return I2CInstance->endTransmission() == 0;
#else
		return true;
#endif		
	}

	// Quick message sender.
	bool SendMessageHeader(const uint8_t header)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Length = 1;

		return WriteCurrentMessage();
	}
};


#endif

