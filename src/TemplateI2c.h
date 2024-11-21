// TemplateI2c.h

#ifndef _TEMPLATE_I2C_h
#define _TEMPLATE_I2C_h

#include <stdint.h>

namespace TemplateI2c
{
	static constexpr size_t I2cAddressMin = 0x0B;
	static constexpr size_t I2cAddressMax = 0x77;

	template<uint8_t payloadSize>
	struct MessageStruct
	{
		uint8_t Header = 0;
		uint8_t Payload[payloadSize]{};
	};

	enum class MessageDefinition
	{
		HeaderIndex = 0,
		PayloadIndex = 1
	};

	static constexpr size_t MessageOverhead = (uint8_t)MessageDefinition::PayloadIndex;
	static constexpr size_t MessageMinSize = MessageOverhead;
	static constexpr size_t MessageMaxSize = 32;
	static constexpr size_t MessageMaxPayloadSize = MessageMaxSize - MessageOverhead;

	static constexpr size_t GetMessageSize(const uint8_t payloadSize)
	{
		return payloadSize + MessageOverhead;
	}

	static void SetUint16(uint8_t* payload, const uint16_t value)
	{
		payload[0] = (uint8_t)(value >> 8);
		payload[1] = (uint8_t)value;
	}

	static constexpr uint16_t GetUint16(const uint8_t* payload)
	{
		return ((uint16_t)payload[1]) | payload[0];
	}

	static void SetUint32(uint8_t* payload, const uint32_t value)
	{
		payload[0] = (uint8_t)(value >> 24);
		payload[1] = (uint8_t)(value >> 16);
		payload[2] = (uint8_t)(value >> 8);
		payload[3] = (uint8_t)value;
	}

	static constexpr uint32_t GetUint32(const uint8_t* payload)
	{
		return ((uint32_t)payload[0] << 24) | ((uint32_t)payload[1] << 16) | ((uint16_t)payload[2] << 8) | payload[3];
	}

	static constexpr size_t GetPayloadSize(const uint8_t messageSize)
	{
		return (messageSize > 0) * (messageSize - MessageOverhead);
	}

	namespace Api
	{
		template<uint8_t header,
			uint8_t payloadSize = 0,
			uint8_t replySize = 0, uint16_t replyDelayMicros = 0>
		struct Request
		{
			static constexpr uint8_t Header = header;
			static constexpr uint8_t PayloadSize = payloadSize;
			static constexpr uint8_t MessageSize = GetMessageSize(payloadSize);
			static constexpr uint8_t ReplySize = replySize;
			static constexpr uint16_t ReplyDelay = replyDelayMicros;
		};

		struct Requests
		{
			/// <summary>
			/// Reset the device.
			/// </summary>
			using Reset = Request<0, 0>;

			/// <summary>
			/// Get the 32 bit device Id.
			/// </summary>
			using GetId = Request<Reset::Header + 1, 0, sizeof(uint32_t), 25>;

			static constexpr uint8_t UserHeaderStart = GetId::Header + 1;
		};
	}
}

#endif