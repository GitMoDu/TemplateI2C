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
	volatile uint8_t Data[MessageSize];

	static const uint8_t Length = MessageSize;

public:
	void Copy(uint8_t* inputRaw, const uint8_t length)
	{
		for (uint8_t i = 0; i < length; i++)
		{
			Data[i] = inputRaw[i];
		}
	}

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

	const uint16_t Get8Bit(const uint8_t offset = 0)
	{
#ifdef I2C_SLAVE_MESSAGE_RANGE_CHECK
		if (offset > Length - sizeof(uint8_t))
		{
			return 0;
		}
#endif

		return Data[offset];
	}

	const uint16_t Get16Bit(const uint8_t offset = 0)
	{
#ifdef I2C_SLAVE_MESSAGE_RANGE_CHECK
		if (offset > Length - sizeof(uint16_t))
		{
			return 0;
		}
#endif
		ArrayToUint16 MessageHelper16;
		MessageHelper16.array[0] = Data[offset];
		MessageHelper16.array[1] = Data[offset + 1];

		return MessageHelper16.uint;
	}

	const bool Set16Bit(const uint16_t value, const uint8_t offset)
	{
#ifdef I2C_SLAVE_MESSAGE_RANGE_CHECK
		if ((offset) > Length)
		{
			return false;
		}
#endif
		ArrayToUint16 MessageHelper16;

		MessageHelper16.uint = value;

		Data[offset] = MessageHelper16.array[0];
		Data[offset + 1] = MessageHelper16.array[1];

		return true;
	}

	const uint32_t Get32Bit(const uint8_t offset = 0)
	{
#ifdef I2C_SLAVE_MESSAGE_RANGE_CHECK
		if (offset > Length - sizeof(uint32_t))
		{
			return 0;
		}
#endif 
		ArrayToUint32 MessageHelper32;

		MessageHelper32.array[0] = Data[offset];
		MessageHelper32.array[1] = Data[offset + 1];
		MessageHelper32.array[2] = Data[offset + 2];
		MessageHelper32.array[3] = Data[offset + 3];

		return MessageHelper32.uint;
	}

	const bool Set32Bit(const uint32_t value, const uint8_t offset)
	{
#ifdef I2C_SLAVE_MESSAGE_RANGE_CHECK
		if ((offset) > Length)
		{
			return false;
		}
#endif
		ArrayToUint32 MessageHelper32;

		MessageHelper32.uint = value;

		Data[offset] = MessageHelper32.array[0];
		Data[offset + 1] = MessageHelper32.array[1];
		Data[offset + 2] = MessageHelper32.array[2];
		Data[offset + 3] = MessageHelper32.array[3];

		return true;
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

	void CopyVariable(uint8_t* inputRaw, const uint8_t length)
	{
		Length = length;

		for (uint8_t i = 0; i < length; i++)
		{
			Data[i] = inputRaw[i];
		}
	}
};
#endif

