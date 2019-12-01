// IMessageI2C.h

#ifndef _IMESSAGEI2C_h
#define _IMESSAGEI2C_h

#include <stdint.h>
//Base interface for I2C message.
class IMessageI2C
{
protected:
	uint8_t Length = 0;

public:
	uint8_t GetLength() { return Length; }

	void Clear()
	{
		Length = 0;
	}

	void SetLength(const uint8_t length)
	{
		Length = length;
	}

	uint8_t GetHeader()
	{
		return GetRaw()[0];
	}

public:
	virtual uint8_t* GetRaw();
	virtual void FastWrite(const uint8_t value);

	///Message reading helpers.
	virtual uint16_t Get8BitPayload(const uint8_t offset = 0);
	virtual uint16_t Get16BitPayload(const uint8_t offset = 0);
	virtual uint32_t Get32BitPayload(const uint8_t offset = 0);
};

#endif

