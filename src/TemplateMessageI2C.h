// TemplateMessageI2C.h

#ifndef _TEMPLATE_MESSAGE_I2C_h
#define _TEMPLATE_MESSAGE_I2C_h

#include <stdint.h>
#include <I2CSlaveDefinitions.h>
#include <MessageHelper.h>
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
#include <IMessageI2C.h>
#endif


//Template class for messages.
template <const uint8_t MessageMaxSize>
class TemplateMessageI2C
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	: public IMessageI2C
#endif // I2C_MESSAGE_IMPLEMENT_INTERFACE
{
protected:
	uint8_t Data[MessageMaxSize];
	uint8_t Length = 0;

public:
	///Overloads.
	void Clear()
	{
		Length = 0;
	}

	uint8_t * GetRaw()
	{
		return &Data[0];
	}

	bool Write(const uint8_t data)
	{
		if (Length < MessageMaxSize)
		{
			Data[Length++] = data;

			return true;
		}

		return false;
	}

	bool Write(uint8_t* source, const uint8_t length)
	{
		if (Length < MessageMaxSize)
		{
			for (uint8_t i = 0; i < length; i++)
			{
				Data[Length++] = source[i];

				if (Length > MessageMaxSize)
				{
					return false;
				}
			}
		}

		return true;
	}

	uint8_t GetLength()
	{
		return Length;

	}
	///End of overloads.

	uint8_t GetHeader()
	{
		return Data[0];
	}


	///Message building helpers.
	void SetHeader(const uint8_t header)
	{
		Data[0] = header;
		if (Length < 1)
		{
			Length = 1;
		}
	}

	uint16_t Get8BitPayload(const uint8_t offset = 0)
	{
		if ((offset  + 1 )> GetLength() - sizeof(uint8_t))
		{
			return 0;
		}

		return Data[(offset  + 1 )];
	}

	uint16_t Get16BitPayload(const uint8_t offset = 0)
	{
		if ((offset  + 1 ) > GetLength() - sizeof(uint16_t))
		{
			return 0;
		}
		MessageHelper16.array[0] = Data[offset + 1];
		MessageHelper16.array[1] = Data[offset + 2];

		return MessageHelper16.uint;
	}

	uint32_t Get32BitPayload(const uint8_t offset = 0)
	{
		if ((offset  + 1 ) > GetLength() - sizeof(uint32_t))
		{
			return 0;
		}

		MessageHelper32.array[0] = Data[offset + 1];
		MessageHelper32.array[1] = Data[offset + 2];
		MessageHelper32.array[2] = Data[offset + 3];
		MessageHelper32.array[3] = Data[offset + 4];

		return MessageHelper32.uint;
	}

	bool Append8BitPayload(const uint8_t value)
	{
		return Write(value);
	}

	bool Append16BitPayload(const uint16_t value)
	{
		MessageHelper16.uint = value;

		if (!Write(MessageHelper16.array, sizeof(uint16_t)))
		{
			return false;
		}

		return true;
	}

	bool Set8BitPayload(const uint8_t value, const uint8_t offset = 0)
	{
		if ((offset  + 1 ) > GetLength() - sizeof(uint8_t))
		{
			return false;
		}

		Data[(offset  + 1 )] = value;

		return true;
	}

	bool Set16BitPayload(const uint16_t value, const uint8_t offset = 0)
	{
		if ((offset  + 1 ) > GetLength() - sizeof(uint16_t))
		{
			return false;
		}

		MessageHelper16.uint = value;

		Data[offset + 1] = MessageHelper16.array[0];
		Data[offset + 2] = MessageHelper16.array[1];

		return true;
	}

	bool Append32BitPayload(const uint32_t value)
	{
		MessageHelper32.uint = value;

		if (!Write(MessageHelper32.array, sizeof(uint32_t)))
		{
			return false;
		}

		return true;
	}

	bool Set32BitPayload(const uint32_t value, const uint8_t offset = 0)
	{
		if ((offset  + 1 ) > GetLength() - sizeof(uint32_t))
		{
			return false;
		}

		MessageHelper32.uint = value;

		Data[offset + 1] = MessageHelper32.array[0];
		Data[offset + 2] = MessageHelper32.array[1];
		Data[offset + 3] = MessageHelper32.array[2];
		Data[offset + 4] = MessageHelper32.array[3];

		return true;
	}
};
#endif

