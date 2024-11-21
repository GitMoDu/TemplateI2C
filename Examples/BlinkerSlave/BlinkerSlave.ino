/* Blinker Template I2C Slave Example
* 
* Implements BlinkerApi over I2C with BlinkerController.
* 
* All message handling is done in interrupt.
*	See BlinkerI2cSlave::OnReceiveInterrupt for example implementation.
* 
*/

#define DEBUG_LOG

#define SERIAL_BAUD_RATE 115200

#define DEBUG_TEMPLATE_I2C

#include <TemplateI2cExample.h>

#include "BlinkerController.h"
#include "BlinkerI2cSlave.h"


BlinkerController Blinker;

BlinkerI2cSlave I2cSlave(Wire, Blinker);


void setup()
{
	Blinker.WriteLed(false);

#ifdef DEBUG_LOG
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;
	Serial.println(F("Template I2C Slave Blinker Device"));
#endif

	if (!I2cSlave.Setup(OnI2CReceive, OnI2CRequest))
	{
#ifdef DEBUG_LOG
		Serial.println(F("Setup Failed."));
#endif
		while (1);;
	}
}

void loop()
{
}

void OnI2CReceive(int length)
{
	I2cSlave.OnReceiveInterrupt(length);
}

void OnI2CRequest()
{
	I2cSlave.OnRequestInterrupt();
}
