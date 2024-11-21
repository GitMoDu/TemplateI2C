// BlinkerI2cSlave.h

#ifndef _BLINKER_I2C_SLAVE_h
#define _BLINKER_I2C_SLAVE_h

#include <TemplateI2cSlave.h>
#include <TemplateI2cExample.h>

#include "BlinkerController.h"

class BlinkerI2cSlave : public TemplateI2c::I2cSlave<BlinkerApi::Address, BlinkerApi::Id>
{
protected:
	using Base = TemplateI2c::I2cSlave<BlinkerApi::Address, BlinkerApi::Id>;
	using Base::Incoming;
	using Base::SetOutMessage;

private:
	BlinkerController& Blinker;

private:
	uint8_t LongReply = 0;

public:
	BlinkerI2cSlave(TwoWire& wire, BlinkerController& blinker)
		: Base(wire)
		, Blinker(blinker)
	{
	}

	void OnReceiveInterrupt(const uint8_t size)
	{
		ReadIncoming(size);

		if (size > 0)
		{
			const uint8_t header = Incoming[(uint8_t)TemplateI2c::MessageDefinition::HeaderIndex];

			switch (header)
			{
			case BlinkerApi::Requests::LedOff::Header:
				Blinker.WriteLed(false);
#ifdef DEBUG_LOG
				Serial.println(F("LED off"));
#endif
				break;
			case BlinkerApi::Requests::LedOn::Header:
				Blinker.WriteLed(true);
#ifdef DEBUG_LOG
				Serial.println(F("LED on"));
#endif
				break;
			case BlinkerApi::Requests::LedToggle::Header:
				Blinker.ToggleLed();
#ifdef DEBUG_LOG
				Serial.println(F("LED toggle"));
#endif
				break;
			case BlinkerApi::Requests::LongRequest::Header:
				delayMicroseconds(BlinkerApi::Requests::LongRequest::ReplyDelay - 50);
				SetOutMessage(&LongReply, sizeof(LongReply));
				break;
			default:
				if (size == TemplateI2c::GetMessageSize(0))
				{
					OnSystemMessageReceived(header);
				}
				break;
			}
		}
	}
};
#endif