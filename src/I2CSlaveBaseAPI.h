// I2CSlaveBaseAPI.h

#ifndef _I2C_SLAVE_BASE_API_h
#define _I2C_SLAVE_BASE_API_h

#include <stdint.h>

#define I2C_BUFFER_SIZE								32
#define TWI_RX_BUFFER_SIZE							( I2C_BUFFER_SIZE )
#define TWI_TX_BUFFER_SIZE							( I2C_BUFFER_SIZE )

#define I2C_ADDRESS_MIN_VALUE						0x10
#define I2C_ADDRESS_MAX_VALUE						0xC0

#ifndef I2C_SLAVE_MAX_MESSAGE_SIZE
#define I2C_SLAVE_MAX_MESSAGE_SIZE 32

// Basic messaging error tracking is nice, but it can be disabled with this property.
//#define I2C_SLAVE_COMMS_ERRORS_ENABLE

// Useful for debugging, overhead when the system is validate.
//#define I2C_SLAVE_MESSAGE_RANGE_CHECK

// If you don't use a task scheduller, all processing will be done in interrupt.
//#define I2C_SLAVE_USE_TASK_SCHEDULER

// Enable device id.
//#define I2C_SLAVE_DEVICE_ID_ENABLE

// Enable device reset.
//#define I2C_SLAVE_DEVICE_RESET_ENABLE

// Enable low power state with static callback.
//#define I2C_SLAVE_DEVICE_LOW_POWER_ENABLE

// Disable actual I2C communication, for testing.
//#define I2C_DRIVER_MOCK_I2C
#endif 

class BaseAPI
{
public:
	template<const uint8_t HeaderValue,
		const uint8_t LengthValue,
		const uint8_t ResponseLengthValue>
		struct ResponseHeader
	{
		static const uint8_t Header = HeaderValue;
		static const uint8_t Length = LengthValue;
		static const uint8_t ResponseLength = ResponseLengthValue;
	};

	template<const uint8_t HeaderValue,
		const uint8_t LengthValue>
		struct CommandHeader
	{
		static const uint8_t Header = HeaderValue;
		static const uint8_t Length = LengthValue;
	};

	static const uint8_t DeviceAddress = 0x88; // R8

	// Common.
	static const uint8_t HeaderMax = 0XEF;
	static const uint8_t HeaderBaseMax = 0XFE;
	static const uint8_t MessageMinMaxSize = sizeof(uint32_t) + 1;
	static const uint8_t MessageMaxSize = I2C_SLAVE_MAX_MESSAGE_SIZE;
	static const uint8_t SizeHeader = 1;

	// Base message headers
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	static const ResponseHeader<HeaderBaseMax - 1,
		SizeHeader,
		SizeHeader + sizeof(uint32_t)> GetDeviceId;
#endif

#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
	static const CommandHeader<HeaderBaseMax - 2,
		SizeHeader> ResetDevice;
#endif

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	static const CommandHeader<HeaderBaseMax - 3,
		SizeHeader> SetLowPowerMode;
#endif
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
	//uint32_t SizeErrors, ContentErrors, Overflows
	static const ResponseHeader<HeaderBaseMax - 4,
		SizeHeader, 
		sizeof(uint32_t) * 3 > GetErrors;
#endif
};
#endif

