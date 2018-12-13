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

	uint8_t GetHeader()
	{
		return Data[0];
	}
	///End of overloads.


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

