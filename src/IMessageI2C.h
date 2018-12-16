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

#endif

