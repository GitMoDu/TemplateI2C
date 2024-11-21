// TemplateI2cExample.h

#ifndef _TEMPLATE_I2C_EXAMPLE_h
#define _TEMPLATE_I2C_EXAMPLE_h

#include <TemplateI2c.h>
#include <Arduino.h>

namespace BlinkerApi
{
	static constexpr uint8_t Address = 123;
	static constexpr uint32_t Id = 123456;

	struct Requests : TemplateI2c::Api::Requests
	{
		using LedOff = TemplateI2c::Api::Request<TemplateI2c::Api::Requests::UserHeaderStart>;
		using LedOn = TemplateI2c::Api::Request<LedOff::Header + 1>;
		using LedToggle = TemplateI2c::Api::Request<LedOn::Header + 1>;
		using LongRequest = TemplateI2c::Api::Request<LedToggle::Header + 1, 0, sizeof(uint8_t), 1000>;
	};
};
#endif