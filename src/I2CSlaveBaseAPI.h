// I2CSlaveBaseAPI.h

#ifndef _I2C_SLAVE_BASE_API_h
#define _I2C_SLAVE_BASE_API_h

#include <stdint.h>

#define I2C_ADDRESS_MIN_VALUE						0x10
#define I2C_ADDRESS_MAX_VALUE						0xC0

#ifndef I2C_SLAVE_MAX_MESSAGE_SIZE
#define I2C_SLAVE_MAX_MESSAGE_SIZE 32
#endif 

// Enable device id.
//#define I2C_SLAVE_DEVICE_ID_ENABLE

// Enable device reset.
//#define I2C_SLAVE_DEVICE_RESET_ENABLE

// Enable low power state with static callback.
//#define I2C_SLAVE_DEVICE_LOW_POWER_ENABLE

// Enable last received timing tracking. Usefull for sleeping after a period of no communications from host.
//#define I2C_SLAVE_DEVICE_TRACK_LAST_RECEIVED_ENABLE

// Basic messaging error flagging, for debugging.
//#define I2C_SLAVE_COMMS_ERRORS_ENABLE

// Disable actual I2C communication, for testing.
//#define I2C_DRIVER_MOCK_I2C


class BaseAPI
{
public:
	union ArrayToUint16 {
		uint8_t array[sizeof(uint16_t)];
		uint16_t uint;
	};

	union ArrayToUint32 {
		uint8_t array[sizeof(uint32_t)];
		uint32_t uint;
	};

	union ArrayToUint64 {
		uint8_t array[sizeof(uint64_t)];
		uint64_t uint;
	};

	static const uint8_t SizeHeader = 1;

	template<const uint8_t HeaderValue,
		const uint8_t LengthValue>
	struct CommandHeader
	{
		static const uint8_t Header = HeaderValue;
		static const uint8_t CommandLength = SizeHeader + LengthValue;
	};

	template<const uint8_t HeaderValue,
		const uint8_t LengthValue,
		const uint8_t ResponseLengthValue>
	struct ResponseHeader : CommandHeader<HeaderValue, LengthValue>
	{
		static const uint8_t ResponseLength = ResponseLengthValue;
	};

public:
	static const uint8_t HeaderMin = 3; // First 3 headers are reserved for template.
	static const uint8_t MessageMaxSize = I2C_SLAVE_MAX_MESSAGE_SIZE;

public:
	// Base message headers
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	using GetDeviceId = ResponseHeader<0, 0, sizeof(uint32_t)>;
#endif

#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
	using ResetDevice = CommandHeader<1, 0>;
#endif

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	 using SetLowPowerMode = CommandHeader<2, 0>;
#endif
};
#endif