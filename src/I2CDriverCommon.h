// I2CDriverCommon.h

#ifndef _I2CDRIVERCOMMON_h
#define _I2CDRIVERCOMMON_h

class I2CDriverCommon
{
public:
	virtual bool CheckDevice() { return false; }
	virtual bool GetTemperature(uint16_t& temperature) { return 0; }
	virtual bool GetVoltage(uint16_t& voltage) { return 0; }
};

#endif

