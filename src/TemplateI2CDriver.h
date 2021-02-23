// TemplateI2CDriver.h

#ifndef _TEMPLATE_I2C_DRIVER_h
#define _TEMPLATE_I2C_DRIVER_h

#include <Arduino.h>
#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>
#include <Wire.h>

/// <summary>
/// DeviceAddress: I2C address.
/// DeviceId: 32 bit device ID.
/// WaitBeforeReadMicros: Minimum time before sequential commands. This depends on the I2C slave hardware and implementation.
/// Defaults to zero so compiler removes it away by default, will throttle sequential commands with blocking delay. 
/// </summary>
template <const uint8_t DeviceAddress,
	const uint32_t DeviceId,
	const uint32_t WaitBeforeReadMicros = 0>
	class TemplateI2CDriver
{
private:
	static const uint8_t SetupRetryMaxCount = 3;

	uint32_t LastCommandMicros = 0;

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

#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
	const bool ResetDevice()
	{
		return SendMessageHeader(BaseAPI::ResetDevice.Header);
	}
#endif

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	const bool SleepDevice()
	{
		return SendMessageHeader(BaseAPI::SetLowPowerMode.Header);
	}
#endif

protected:
	const bool WriteCurrentMessage()
	{
#ifndef I2C_DRIVER_MOCK_I2C
		CoalesceDelay();

		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write(OutgoingMessage.Data, OutgoingMessage.Length);

		if (I2CInstance->endTransmission() == 0)
		{
			if (WaitBeforeReadMicros > 0)
			{
				LastCommandMicros = micros();
			}
			return true;
		}
		else
		{
			return false;
		}
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

		CoalesceDelay();

		if (I2CInstance->requestFrom(DeviceAddress, requestSize))
		{
			while (I2CInstance->available())
			{
				IncomingMessage.FastWrite(I2CInstance->read());
			}

			if (WaitBeforeReadMicros > 0)
			{
				LastCommandMicros = micros();
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

	void CoalesceDelay()
	{
		if (WaitBeforeReadMicros > 0)
		{
			uint32_t delta = micros() - LastCommandMicros;

			if (delta < WaitBeforeReadMicros &&
				WaitBeforeReadMicros - delta > 0)
			{
				delayMicroseconds(WaitBeforeReadMicros - delta);
			}
		}
	}
};
#endif