// TemplateMessageI2C.h

#ifndef _TEMPLATE_MESSAGE_I2C_h
#define _TEMPLATE_MESSAGE_I2C_h

#include <I2CSlaveBaseAPI.h>


// Template class for generic message.
template<const uint8_t MessageSize>
class TemplateMessageI2C
{
private:
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

public:
	uint8_t Data[MessageSize];

	static const uint8_t Length = MessageSize;

public:
	// Header helpers.
	const uint8_t GetHeader()
	{
		return Data[0];
	}

	void SetHeader(const uint8_t header)
	{
		Data[0] = header;
	}

	uint8_t* GetPayload()
	{
		return &Data[1];
	}

	const uint16_t Get16Bit(const uint8_t offset = 0)
	{
		ArrayToUint16 MessageHelper16;
		MessageHelper16.array[0] = Data[offset];
		MessageHelper16.array[1] = Data[offset + 1];

		return MessageHelper16.uint;
	}

	void Set16Bit(const uint16_t value, const uint8_t offset)
	{
		ArrayToUint16 MessageHelper16;

		MessageHelper16.uint = value;

		Data[offset] = MessageHelper16.array[0];
		Data[offset + 1] = MessageHelper16.array[1];
	}

	const uint32_t Get32Bit(const uint8_t offset = 0)
	{
		ArrayToUint32 MessageHelper32;

		MessageHelper32.array[0] = Data[offset];
		MessageHelper32.array[1] = Data[offset + 1];
		MessageHelper32.array[2] = Data[offset + 2];
		MessageHelper32.array[3] = Data[offset + 3];

		return MessageHelper32.uint;
	}

	void Set32Bit(const uint32_t value, const uint8_t offset)
	{
		ArrayToUint32 MessageHelper32;

		MessageHelper32.uint = value;

		Data[offset] = MessageHelper32.array[0];
		Data[offset + 1] = MessageHelper32.array[1];
		Data[offset + 2] = MessageHelper32.array[2];
		Data[offset + 3] = MessageHelper32.array[3];
	}
};

// Template class for buffering message.
template<const uint8_t MessageSize>
class TemplateVariableMessageI2C : public TemplateMessageI2C<MessageSize>
{
public:
	using TemplateMessageI2C<MessageSize>::Data;
	volatile uint8_t Length = 0;

public:
	void Clear()
	{
		Length = 0;
	}

	// Used for streaming from I2C buffer.
	void FastWrite(const uint8_t value)
	{
		Data[Length++] = value;
	}
};
#endif

