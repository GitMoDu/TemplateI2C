// Platform.h

#ifndef _TEMPLATE_I2C_PLATFORM_AVR_h
#define _TEMPLATE_I2C_PLATFORM_AVR_h

#include <Arduino.h>

namespace TemplateI2c
{
	namespace Platform
	{
#if defined(ARDUINO_ARCH_AVR)
		void(*ResetCall) (void) = 0;
#else
		void ResetCall() {}
#endif
	}
}
#endif