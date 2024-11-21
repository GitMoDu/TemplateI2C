// BlinkerI2cDriver.h

#ifndef _BLINKER_I2C_DRIVER_h
#define _BLINKER_I2C_DRIVER_h

#include <TemplateAsyncI2cDriver.h>
#include <TemplateI2cExample.h>


class BlinkerI2cAsyncDriver : public TemplateI2c::AsyncDriver::I2cDriver<BlinkerApi::Address, BlinkerApi::Id>
{
private:
	using Base = TemplateI2c::AsyncDriver::I2cDriver<BlinkerApi::Address, BlinkerApi::Id>;

protected:
	using Base::RequestResponse;
	using Base::OnI2cReceived;

protected:
	using Base::LargestDelay;
	using Base::ReplyMinDelay;

private:
	void (*OnLongRequestResult)(const uint8_t result) = nullptr;

public:
	BlinkerI2cAsyncDriver(TS::Scheduler& scheduler, TwoWire& wire)
		: Base(scheduler, wire)
	{
	}

public:
	virtual void OnI2cError(const TemplateI2c::AsyncDriver::ErrorEnum error) final
	{
		Serial.print(F("I2C Error: "));
		switch (error)
		{
		case TemplateI2c::AsyncDriver::ErrorEnum::InvalidSize:
			Serial.println(F("InvalidSize"));
			break;
		case TemplateI2c::AsyncDriver::ErrorEnum::ReadOverflow:
			Serial.println(F("ReadOverflow"));
			break;
		case TemplateI2c::AsyncDriver::ErrorEnum::RequestFail:
			Serial.println(F("RequestFail"));
			break;
		default:
			break;
		}
	}

	virtual void OnI2cReceived(const uint8_t requestHeader, const uint8_t size) final
	{
		if (requestHeader == BlinkerApi::Requests::LongRequest::Header
			&& size == BlinkerApi::Requests::LongRequest::ReplySize
			&& OnLongRequestResult != nullptr)
		{
			OnLongRequestResult(Incoming[0]);
		}
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

	const bool LongRequestAsync(void (*onLongRequestResult)(const uint8_t result))
	{
		OnLongRequestResult = onLongRequestResult;

		return RequestResponse(BlinkerApi::Requests::LongRequest::Header,
			BlinkerApi::Requests::LongRequest::ReplySize,
			BlinkerApi::Requests::LongRequest::ReplyDelay);
	}
};
#endif