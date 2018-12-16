// MessageHelper.h

#ifndef _MESSAGEHELPER_h
#define _MESSAGEHELPER_h

#include <stdint.h>

#include <IMessageI2C.h>

static union ArrayToUint16 {
	uint8_t array[2];
	uint16_t uint;
}MessageHelper16;

static union ArrayToUint32 {
	uint8_t array[4];
	uint32_t uint;
} MessageHelper32;

#endif

