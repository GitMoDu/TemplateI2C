// BlinkerI2cDriver.h

#ifndef _BLINKER_I2C_DRIVER_h
#define _BLINKER_I2C_DRIVER_h

#include <TemplateI2cDriver.h>
#include <TemplateI2cExample.h>

class BlinkerI2cDriver : public TemplateI2c::I2cDriver<BlinkerApi::Address, BlinkerApi::Id, BlinkerApi::ReplyMinDelay>
{
private:
	using Base = TemplateI2c::I2cDriver<BlinkerApi::Address, BlinkerApi::Id, BlinkerApi::ReplyMinDelay>;

protected:
	using Base::LargestDelay;
	using Base::ReplyMinDelay;

public:
	BlinkerI2cDriver(TwoWire& wire)
		: Base(wire)
	{
	}

public:
	void LedOff()
	{
		SendMessage(BlinkerApi::Requests::LedOff::Header);
	}

	void LedOn()
	{
		SendMessage(BlinkerApi::Requests::LedOn::Header);
	}

	void LedToggle()
	{
		SendMessage(BlinkerApi::Requests::LedToggle::Header);
	}

	const uint8_t LongRequest()
	{
		if (SendMessage(BlinkerApi::Requests::LongRequest::Header))
		{
			delayMicroseconds(LargestDelay(BlinkerApi::Requests::LongRequest::ReplyDelay, ReplyMinDelay));
			if (GetResponse(BlinkerApi::Requests::LongRequest::ReplySize))
			{
				return Incoming[0];
			}
		}

		return 0;
	}
};
#endif