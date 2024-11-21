// TemplateI2cDriver.h

#ifndef _TEMPLATE_I2_CDRIVER_h
#define _TEMPLATE_I2_CDRIVER_h

#include "TemplateI2c.h"
#include <Wire.h>

namespace TemplateI2c
{
	template<const uint8_t pinSCL,
		const uint8_t pinSDA,
		const uint8_t i2cChannel,
		const uint32_t i2cSpeed>
	static const bool SetupI2C(TwoWire& wire)
	{
#if defined(ARDUINO_ARCH_ESP32)
		return wire.begin(pinSDA, pinSCL, i2cSpeed);
#else
		wire.begin();

		if (i2cSpeed > 0)
		{
			wire.setClock(i2cSpeed);
		}

#if defined(ARDUINO_ARCH_NRF52)
		if (pinSDA != UINT8_MAX
			&& pinSCL != UINT8_MAX)
		{
			wire.setPins(pinSDA, pinSCL);
		}
#endif

		return true;
#endif
	}

	template<const uint8_t address, const uint32_t id>
	class I2cDriver
	{
	public:
		static constexpr uint8_t Address = address;

	protected:
		TwoWire& WireInstance;

	protected:
		uint8_t Incoming[TemplateI2c::MessageMaxSize]{};

	public:
		I2cDriver(TwoWire& wire)
			: WireInstance(wire)
		{
		}

		const bool ResetDevice()
		{
			return SendMessage(Api::Requests::Reset::Header);
		}

		const bool GetDeviceId(uint32_t& deviceId)
		{
			if (SendMessage(Api::Requests::GetId::Header))
			{
				delayMicroseconds(Api::Requests::GetId::ReplyDelay);
				if (GetResponse(Api::Requests::GetId::ReplySize))
				{
					deviceId = GetUint32(Incoming);

					return true;
				}
			}

			return false;
		}

		const bool CheckDevicePresent()
		{
			uint32_t deviceId = 0;

			return GetDeviceId(deviceId) && id == deviceId;
		}

	protected:
		const bool SendMessage(uint8_t* message, const uint8_t size)
		{
			Wire.beginTransmission(address);
			Wire.write(message, size);

			return Wire.endTransmission() == 0;
		}

		const bool SendMessage(const uint8_t header, const uint8_t* payload, const uint8_t payloadSize)
		{
			Wire.beginTransmission(address);
			Wire.write(header);
			Wire.write(payload, payloadSize);

			return Wire.endTransmission() == 0;
		}

		const bool SendMessage(const uint8_t header)
		{
			Wire.beginTransmission(address);
			Wire.write(header);

			return Wire.endTransmission() == 0;
		}

	protected:
		const bool GetResponse(const uint8_t responseSize)
		{
			uint_fast8_t size = 0;
			if (Wire.requestFrom(address, responseSize))
			{
				while (Wire.available())
				{
					Incoming[size++] = Wire.read();
				}

				return size == responseSize;
			}

			return false;
		}
	};
}
#endif
