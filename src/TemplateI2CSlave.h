// TemplateI2CSlave.h

#ifndef _TEMPLATE_I2C_SLAVE_h)
#define _TEMPLATE_I2C_SLAVE_h

#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>

#include <Arduino.h>
#include <Wire.h>

template<const uint8_t DeviceAddress,
	const uint32_t DeviceId>
	class TemplateI2CSlave
{
private:
	// I2C Read pointer output, source data is always external.
	uint8_t* OutgoingPointer = nullptr;
	volatile uint8_t OutgoingSize = 0;
	//

	// Base Slave info messages.
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
	TemplateMessageI2C<BaseAPI::GetDeviceId.ResponseLength> IdMessage;
#endif

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
	TemplateMessageI2C<BaseAPI::GetErrors.ResponseLength> ErrorsMessage;
#endif
	//

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	// External low power handling.
	void (*LowPowerFunction)() = nullptr;
	//
#endif

protected:
	// Buffered read message.
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> IncomingProcessingMessage;

protected:
	virtual bool ProcessMessage() { return false; }

	virtual bool SetupWireCallbacks()
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
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		, ErrorsMessage()
#else
		: ErrorsMessage()
#endif

#endif
	{
	}

	const uint32_t GetDeviceId() { return DeviceId; }

	virtual bool Setup()
	{
		if (DeviceAddress > I2C_ADDRESS_MIN_VALUE
			&& DeviceAddress < I2C_ADDRESS_MAX_VALUE
#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
			&& LowPowerFunction != nullptr
#endif
			&& PrepareBaseMessages())
		{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
			ErrorsMessage.Set32Bit(0, 0);
			ErrorsMessage.Set32Bit(0, sizeof(uint32_t));
			ErrorsMessage.Set32Bit(0, sizeof(uint32_t) * 2);
#endif

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


#ifdef DEBUG_TEMPLATE_I2C
	void TestInputMessage(uint8_t* data, const uint8_t length)
	{
		IncomingProcessingMessage.CopyVariable(data, length);
		InInterruptMessageProcessing();
	}
#endif

#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
	void SetLowPowerFunction(void (*lowPowerFunction)())
	{
		LowPowerFunction = lowPowerFunction;
	}
#endif

	void OnReceive(int16_t length)
	{
		if (length < 1 ||
			length > min(BaseAPI::MessageMaxSize, TWI_RX_BUFFER_SIZE))
		{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
			ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(0) + 1, 0);
#endif
			while (length--)
			{
				Wire.read();
			}

			return;
		}

		IncomingProcessingMessage.Clear();
		while (length--)
		{
			IncomingProcessingMessage.FastWrite(Wire.read());
		}
		InInterruptMessageProcessing();
	}

	void OnRequest()
	{
		// Outgoing is always valid and ready to send.
		Wire.write(OutgoingPointer, OutgoingSize);
	}

	void InInterruptMessageProcessing()
	{
		if (IncomingProcessingMessage.Length > 0)
		{
			// Process message with base headers.
			if (!ProcessMessageInternal())
			{
				// Process implementation messages.
				if (!ProcessMessage())
				{
					// Unrecognized message.
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
					ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(sizeof(uint32_t)) + 1, sizeof(uint32_t));
#endif
				}
			}
		}
	}

protected:
	void SetOutput(volatile uint8_t* output, const uint8_t length)
	{
		if (output != nullptr && length <= TWI_TX_BUFFER_SIZE)
		{
			OutgoingPointer = output;
			OutgoingSize = length;
		}
	}

private:
	bool PrepareBaseMessages()
	{
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		IdMessage.SetHeader(BaseAPI::GetDeviceId.Header);
		IdMessage.Set32Bit(GetDeviceId(), BaseAPI::SizeHeader);
#endif
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		ErrorsMessage.Set32Bit(0, 0);
		ErrorsMessage.Set32Bit(0, sizeof(uint32_t));
		ErrorsMessage.Set32Bit(0, sizeof(uint32_t) * 2);
#endif
		return true;
	}

#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
#if !defined(__AVR__)
#error ONLY AVR SUPPORTS RESET
#endif
	void(*ResetDevice) (void) = 0;
#endif

	bool ProcessMessageInternal()
	{
		switch (IncomingProcessingMessage.GetHeader())
		{
#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		case BaseAPI::GetDeviceId.Header:
			if (IncomingProcessingMessage.Length == BaseAPI::GetDeviceId.CommandLength)
			{
#ifdef DEBUG_TEMPLATE_I2C
				Serial.println(F("GetId"));
#endif
				OutgoingPointer = IdMessage.Data;
				OutgoingSize = BaseAPI::GetDeviceId.ResponseLength;
			}
			else
			{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
				ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(0) + 1, 0);
#endif
			}
			return true;
#endif
#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
		case BaseAPI::ResetDevice.Header:
			if (IncomingProcessingMessage.Length == BaseAPI::ResetDevice.CommandLength)
			{
#ifdef DEBUG_TEMPLATE_I2C
				Serial.println(F("Reset device, bye bye!"));
#endif
				ResetDevice();
				// Never runs.
			}
			else
			{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
				ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(0) + 1, 0);
#endif
			}
			return true;
#endif
#ifdef I2C_SLAVE_DEVICE_LOW_POWER_ENABLE
		case BaseAPI::SetLowPowerMode.Header:
			if (IncomingProcessingMessage.Length == BaseAPI::SetLowPowerMode.CommandLength)
			{
#ifdef DEBUG_TEMPLATE_I2C
				Serial.println(F("Sleep device, be back on interrupt."));
#endif
				// If we're processing this mid interrupt, clear pending Wire messages.
				Wire.flush();

				LowPowerFunction();
			}
			else
			{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
				ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(0) + 1, 0);
#endif
			}
			return true;
#endif
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		case BaseAPI::GetErrors.Header:
			if (IncomingProcessingMessage.Length == BaseAPI::GetErrors.CommandLength)
			{
				OutgoingPointer = ErrorsMessage.Data;
				OutgoingSize = ErrorsMessage.Length;
			}
			else
			{
				// Ironic. Couldn't report on the errors, so we just keep counting errors.
				ErrorsMessage.Set32Bit(ErrorsMessage.Get32Bit(0) + 1, 0);
			}
			return true;
#endif
		default:
			break;
		}

		return false;
	}
};
#endif

