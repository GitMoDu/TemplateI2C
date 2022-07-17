// ExampleApi.h

#ifndef _EXAMPLE_API_h
#define _EXAMPLE_API_h


#define I2C_SLAVE_MAX_MESSAGE_SIZE 32

#define I2C_SLAVE_COMMS_ERRORS_ENABLE
#define I2C_SLAVE_MESSAGE_RANGE_CHECK
//#define I2C_SLAVE_USE_TASK_SCHEDULER
#define I2C_SLAVE_DEVICE_ID_ENABLE
//#define I2C_SLAVE_DEVICE_RESET_ENABLE
//#define I2C_SLAVE_DEVICE_LOW_POWER_ENABLE

#include <I2CSlaveBaseAPI.h>

class ExampleApi : public BaseAPI
{
public:
	const static uint32_t DeviceId = 00001;
	const static uint8_t DeviceAddress = 0x25;

	static const uint32_t MillisBeforeResponse = 1;

	// Message headers.
	using Start = CommandHeader<0x0F, SizeHeader>;

	using Stop = CommandHeader<0x1F, SizeHeader>;
};
#endif

