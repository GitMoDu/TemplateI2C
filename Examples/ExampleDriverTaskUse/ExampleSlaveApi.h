// I2CServosSlaveApi.h

#ifndef _I2CSERVOSSLAVEAPI_h
#define _I2CSERVOSSLAVEAPI_h

#include <stdint.h>
#include <I2CSlaveConstants.h>

class ExampleApi
{
public:
	enum RequestHeader : uint8_t
	{
		Start = 1,
		Stop = 2,
		GetId = I2C_SLAVE_BASE_HEADER_DEVICE_ID,
		GetSerial = I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL,
	};

	const static uint32_t DeviceId = 00001;

	const static uint8_t DeviceAddress = 0x25;

	const static uint32_t MicrosBetweenMessages = 200;

};
#endif

