// TemplateI2CSlave.h

#ifndef _TEMPLATE_I2C_SLAVE_h)
#define _TEMPLATE_I2C_SLAVE_h

#ifdef ATTINY_CORE
#define I2C_BUFFER_SIZE								32
#define TWI_RX_BUFFER_SIZE							( I2C_BUFFER_SIZE )
#define TWI_TX_BUFFER_SIZE							( I2C_BUFFER_SIZE )
#endif

#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>

#include <Arduino.h>
#include <Wire.h>


template<const uint8_t DeviceAddress
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	, const uint32_t DeviceId
#endif
>
class TemplateI2CSlave
{
protected:
	// I2C Read pointer output, source data is always external.
	volatile uint8_t OutgoingSize = 0;
	uint8_t* Outgoing = nullptr;
	//

#ifdef I2C_SLAVE_DEVICE_TRACK_LAST_RECEIVED_ENABLE
	volatile uint32_t LastReceived = 0;
#endif

public:
	// Base Slave info messages.
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	TemplateMessageI2C<BaseAPI::GetDeviceId.ResponseLength> IdMessage;
#endif
	//

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	// External low power handling.
	void (*LowPowerFunction)() = nullptr;
	//
#endif

#ifdef DEBUG_TEMPLATE_I2C
	// Debug flag, useful for lighting up an LED when an error occurs.
	volatile ErrorFlag = false;
#endif

protected:
	// Buffered read message.
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> IncomingProcessingMessage;

protected:
	virtual void ProcessMessage() {}

	virtual const bool SetupWireCallbacks()
	{
		// Join i2c bus with address and attach interrupt callbacks.
		// This should be done from last inheriting class, 
		// to avoid virtual overhead during interrupt.
		//Wire.onReceive(ReceiveEvent);
		//Wire.onRequest(RequestEvent);
		return false;
	}

public:
	TemplateI2CSlave()
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		: IdMessage()
#endif
	{
	}

#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	const uint32_t GetDeviceId() { return DeviceId; }
#endif

	virtual const bool Setup()
	{
		if (DeviceAddress >= I2C_ADDRESS_MIN_VALUE
			&& DeviceAddress < I2C_ADDRESS_MAX_VALUE
#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
			&& LowPowerFunction != nullptr
#endif
			&& PrepareBaseMessages())
		{
			// Overzealous I2C Setup.
#ifdef ATTINY_CORE
			pinMode(PIN_USI_SCL, INPUT);
			pinMode(PIN_USI_SDA, INPUT);
#else
			pinMode(PIN_WIRE_SCL, INPUT);
			pinMode(PIN_WIRE_SDA, INPUT);
#endif
			delay(1);
			Wire.flush();
			delay(1);
			//

			SetupWireCallbacks();

			Wire.begin(DeviceAddress);

			return true;
		}

		return false;
	}

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	void SetLowPowerFunction(void (*lowPowerFunction)())
	{
		LowPowerFunction = lowPowerFunction;
	}
#endif

#ifdef I2C_SLAVE_DEVICE_TRACK_LAST_RECEIVED_ENABLE
	const uint32_t GetLastReceivedElapsedMillis()
	{
		return millis() - LastReceived;
	}
#endif

	void OnReceive(int16_t length)
	{
		// Validate length.
		if (length < 1
#ifdef DEBUG_TEMPLATE_I2C
			length > BaseAPI::MessageMaxSize)
#else
			)
#endif
		{
			while (length--)
			{
				Wire.read();
			}

#ifdef DEBUG_TEMPLATE_I2C
			ErrorFlag = true;
#endif
			return;
		}

		IncomingProcessingMessage.Clear();
		while (length--)
		{
			IncomingProcessingMessage.FastWrite(Wire.read());
		}

		ProcessMessageInternal();

#ifdef I2C_SLAVE_DEVICE_TRACK_LAST_RECEIVED_ENABLE
		LastReceived = millis();
#endif
	}

	void OnRequest()
	{
		// Outgoing is always valid and ready to send.
		Wire.write(Outgoing, OutgoingSize);
#ifdef I2C_SLAVE_DEVICE_TRACK_LAST_RECEIVED_ENABLE
		LastReceived = millis();
#endif
	}

public:
#if !defined(__AVR__)
#error ONLY AVR SUPPORTS RESET
#endif
	void(*ResetDevice) (void) = 0;

protected:
#ifdef DEBUG_TEMPLATE_I2C
	void SetOutput(uint8_t* output, const uint8_t length)
	{
		if (output != nullptr && length > 0 && length <= BaseAPI::MessageMaxSize)
		{
			Outgoing = output;
			OutgoingSize = length;
		}
		else
		{
			ErrorFlag = true;
		}
	}
#endif

private:
	const bool PrepareBaseMessages()
	{
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		IdMessage.Set32Bit(GetDeviceId(), 0);
#endif
		return true;
	}

	void ProcessMessageInternal()
	{
		switch (IncomingProcessingMessage.GetHeader())
		{
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		case BaseAPI::GetDeviceId.Header:
#ifdef DEBUG_TEMPLATE_I2C
			ErrorFlag |= IncomingProcessingMessage.Length != BaseAPI::GetDeviceId.CommandLength;
#endif
			Outgoing = IdMessage.Data;
			OutgoingSize = IdMessage.ResponseLength;
			return;
#endif
#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
		case BaseAPI::ResetDevice.Header:
#ifdef DEBUG_TEMPLATE_I2C
			ErrorFlag |= IncomingProcessingMessage.Length != BaseAPI::ResetDevice.CommandLength;
#endif
			// If we're processing this mid interrupt, clear pending Wire messages.
			Wire.flush();
			ResetDevice();
			// Never runs.

			return;
#endif
#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
		case BaseAPI::SetLowPowerMode.Header:
#ifdef DEBUG_TEMPLATE_I2C
			ErrorFlag |= IncomingProcessingMessage.Length != BaseAPI::SetLowPowerMode.CommandLength;
#endif

			// Sleep device, be back on interrupt.
			// If we're processing this mid interrupt, clear pending Wire messages.
			Wire.flush();
			LowPowerFunction();

			return;
#endif
		default:
			ProcessMessage();
			break;
		}
	}
};
#endif