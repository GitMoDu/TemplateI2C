// IMessageI2C.h

#ifndef _IMESSAGEI2C_h
#define _IMESSAGEI2C_h

#include <stdint.h>

//Base interface for I2C message.
class IMessageI2C
{
public:
	virtual void Clear() {};
	virtual uint8_t * GetRaw() { return nullptr; }
	virtual bool Write(const uint8_t data) { return false; }
	virtual bool Write(uint8_t* source, const uint8_t length) { return false; }
	virtual uint8_t GetLength() { return 0; }
};

//Template class for messages.
#define I2C_SLAVE_BASE_HEADER						0xF0
#define I2C_SLAVE_MAX_HEADER_VALUE					( I2C_SLAVE_BASE_HEADER - 1 )
#define I2C_SLAVE_BASE_HEADER_DEVICE_ID				( I2C_SLAVE_BASE_HEADER + 0)
#define I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL			( I2C_SLAVE_BASE_HEADER + 1)
#define I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_REPORT	( I2C_SLAVE_BASE_HEADER + 2)
union ArrayToUint32 {
	byte array[4];
	uint32_t uint;
};

union ArrayToUint16 {
	byte array[2];
	uint16_t uint;
};

template <const uint8_t MessageMaxSize>
class TemplateMessageI2C : public IMessageI2C
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
};
#endif

