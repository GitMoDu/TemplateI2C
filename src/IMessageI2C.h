// IMessageI2C.h

#ifndef _IMESSAGEI2C_h
#define _IMESSAGEI2C_h

#include <stdint.h>
//Base interface for I2C message.
class IMessageI2C
{
public:
	virtual void Clear();
	virtual uint8_t * GetRaw();
	virtual bool Write(const uint8_t data);
	virtual bool Write(uint8_t* source, const uint8_t length);
	virtual uint8_t GetLength();

	///Message building helpers.
	virtual uint8_t GetHeader();
	virtual void SetHeader(const uint8_t header);

	virtual uint16_t Get8BitPayload(const uint8_t offset = 0);
	virtual uint16_t Get16BitPayload(const uint8_t offset = 0);
	virtual uint32_t Get32BitPayload(const uint8_t offset = 0);

	virtual bool Append8BitPayload(const uint8_t value);
	virtual bool Append16BitPayload(const uint16_t value);
	virtual bool Append32BitPayload(const uint32_t value);

	virtual bool Set8BitPayload(const uint8_t value, const uint8_t offset = 0);
	virtual bool Set16BitPayload(const uint16_t value, const uint8_t offset = 0);
	virtual bool Set32BitPayload(const uint32_t value, const uint8_t offset = 0);
};

#endif

