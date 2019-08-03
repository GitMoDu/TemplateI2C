// AbstractI2CSlave.h

#if !defined(_ABSTRACTI2CSLAVE_h) && defined(__AVR__)
#define _ABSTRACTI2CSLAVE_h

#define DEBUG_ABSTRACT_I2CSLAVE
#ifdef DEBUG_ABSTRACT_I2CSLAVE
#define DebugSerial Serial
#endif

#define ABSTRACT_I2C_SLAVE_USE_OUTPUT_CHECK

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>
#include <RingBufCPP.h>
#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>
#include <Wire.h>

class I2CInterruptTask
#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	: public Task
#endif
{
protected:
	volatile uint32_t LastI2CEventMillis = 0;

public:
	virtual void OnReceive(int length) {}
	virtual void OnRequest() {}

#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	I2CInterruptTask(Scheduler* scheduler) : Task(0, TASK_FOREVER, scheduler, true)
#else 
	I2CInterruptTask()
#endif
	{
	}

	inline void TimeStampI2CEvent()
	{
		LastI2CEventMillis = millis();
	}
};

I2CInterruptTask* I2CHandler = nullptr;

void ReceiveEvent(int length)
{
	I2CHandler->OnReceive(length);
}

void RequestEvent()
{
	I2CHandler->OnRequest();
}

template <const uint8_t DeviceAddress, const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN, const uint8_t ReceiverQueueDepth = I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH>
class AbstractI2CSlaveTask : public I2CInterruptTask
{
private:
	///Message queue.
	volatile bool IncomingMessageAvailable = false;
	RingBufCPP<TemplateMessageI2C<MessageMaxSize>, ReceiverQueueDepth> MessageQueue;
	TemplateMessageI2C<MessageMaxSize> IncomingMessage;
	TemplateMessageI2C<MessageMaxSize> CurrentMessage;
	///

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
	///Error messages.
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsOverflowMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsBadSizeMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsContentMessage;
#else
	TemplateMessageI2C<MessageMaxSize> MessageErrorsOverflowMessage;
	TemplateMessageI2C<MessageMaxSize> MessageErrorsBadSizeMessage;
	TemplateMessageI2C<MessageMaxSize> MessageErrorsContentMessage;
#endif
	///

	///Error and Status for this session.
	volatile uint32_t MessageOverflows = 0;
	volatile uint32_t MessageSizeErrors = 0;
	volatile uint32_t MessageContentErrors = 0;
	volatile bool MessageErrorReportNeedsUpdating = false;
	///
#endif

#ifdef I2C_SLAVE_HEALTH_ENABLE
	///Health for this session.
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_8BIT_X1> HealthReportMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_16BIT_X1> VoltageMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_16BIT_X1> TemperatureMessage;
	///
#else
	TemplateMessageI2C<MessageMaxSize> HealthReportMessage;
	TemplateMessageI2C<MessageMaxSize> VoltageMessage;
	TemplateMessageI2C<MessageMaxSize> TemperatureMessage;
#endif
#endif
	///Slave info messages.
	TemplateMessageI2C<MessageMaxSize> IdMessage;
	TemplateMessageI2C<MessageMaxSize> SerialMessage;
	///

private:
#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
#if !defined(__AVR__)
#error ONLY AVR SUPPORTS RESET
#endif
	void(*ResetDevice) (void) = 0;
#endif

protected:
	///I2C Read output message.

	//TemplateMessageI2C<MessageMaxSize>* OutgoingMessage = nullptr;
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	IMessageI2C* OutgoingMessage = nullptr;
#else
	TemplateMessageI2C<MessageMaxSize>* OutgoingMessage = nullptr;
#endif // I2C_MESSAGE_IMPLEMENT_INTERFACE


protected:
#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	virtual bool ProcessMessage(IMessageI2C* currentMessage) {}
#else
	virtual bool ProcessMessage(TemplateMessageI2C<MessageMaxSize>* currentMessage) { return false; }
#endif // I2C_MESSAGE_IMPLEMENT_INTERFACE

	virtual bool OnSetup() { return true; }
	virtual uint32_t GetDeviceId() { return 0; }
	virtual uint32_t GetSerial() { return 0; }

protected:
	void OnMessageOverflowError()
	{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		MessageOverflows++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
#endif
	}

	void OnMessageSizeError()
	{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		MessageSizeErrors++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
#endif
	}

	void OnMessageContentError()
	{
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		MessageContentErrors++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
#endif
	}

#ifdef I2C_MESSAGE_IMPLEMENT_INTERFACE
	void SetOutputMessage(IMessageI2C* outputMessage)
#else
	void SetOutputMessage(TemplateMessageI2C<MessageMaxSize>* outputMessage)
#endif
	{
#ifdef ABSTRACT_I2C_SLAVE_USE_OUTPUT_CHECK
		if (outputMessage != nullptr)//TODO: remove check optional.
#endif
		{
			OutgoingMessage = outputMessage;
		}
	}

public:
#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	AbstractI2CSlaveTask(Scheduler* scheduler) : I2CInterruptTask(scheduler)
#else
	AbstractI2CSlaveTask() : I2CInterruptTask()
#endif	
	{
		I2CHandler = this;
	}

	bool Setup()
	{
		if (I2CHandler != nullptr &&
			DeviceAddress > I2C_ADDRESS_MIN_VALUE
			&& DeviceAddress < I2C_ADDRESS_MAX_VALUE)
		{
			///Overzealous I2C Setup.
			pinMode(PIN_WIRE_SCL, INPUT);
			pinMode(PIN_WIRE_SDA, INPUT);
			delay(1);
			Wire.flush();
			delay(1);
			///

			Wire.begin(DeviceAddress); //Join i2c bus with address.
			Wire.onReceive(ReceiveEvent);
			Wire.onRequest(RequestEvent);

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
			MessageOverflows = 0;
			MessageSizeErrors = 0;
			MessageContentErrors = 0;
#endif

			if (PrepareBaseMessages())
			{
				LastI2CEventMillis = 0;

				return OnSetup();
			}
		}
		else
		{
			Wire.onReceive(nullptr);
			Wire.onRequest(nullptr);
		}

		return false;
	}

	void AddForceMessage(TemplateMessageI2C<MessageMaxSize>* message)
	{
		MessageQueue.addForce(*message);
		enable();
	}

	void AddMessage(TemplateMessageI2C<MessageMaxSize>* message)
	{
		IncomingMessage = *message;
		IncomingMessageAvailable = true;
		enable();
	}

#ifdef I2C_SLAVE_HEALTH_ENABLE
	void SetHealthReportCode(const uint8_t healthCode)
	{
		HealthReportMessage.Set8BitPayload(healthCode);
	}

	void SetHealthVoltage(const uint16_t milliVolts)
	{
		VoltageMessage.Set16BitPayload(milliVolts);
	}

	void SetHealthTemperature(const uint16_t milliDegreesCelsius)
	{
		TemperatureMessage.Set16BitPayload(milliDegreesCelsius);
	}
#endif

#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	bool OnEnable()
	{
		return true;
	}
#else
	void enable() {};
	void enableIfNot() {};
	void disable() {};
#endif	

	void OnReceive(int length)
	{
		if (length < 1 ||
			length > TWI_RX_BUFFER_SIZE)
		{
			//Sanity-check.
			OnMessageSizeError();

			return;
		}

		if (IncomingMessageAvailable)
		{
			//Cannot respond so quickly, hold your horses.
			//If this happens, we've skipped a message.
			OnMessageOverflowError();

			return;
		}

		IncomingMessage.Clear();
		//We copy to a second buffer, so we can process it in the main loop safely, instead of in the interrupt.
		while (length--)
		{
			if (!IncomingMessage.Append(Wire.read()))
			{
				OnMessageSizeError();

				return;
			}
		}

		IncomingMessageAvailable = true;
		enable();
		TimeStampI2CEvent();
	}

	void OnRequest()
	{
#ifdef ABSTRACT_I2C_SLAVE_USE_OUTPUT_CHECK
		if (OutgoingMessage != nullptr)//TODO: remove check optional.
#endif
		{
			Wire.write(OutgoingMessage->GetRaw(), (size_t)min(TWI_TX_BUFFER_SIZE, OutgoingMessage->GetLength()));
		}

		TimeStampI2CEvent();
	}

	bool Callback()
	{
		//The copy buffer operations is staggered, to allow for queue fill without disrupting interrupts.
		if (IncomingMessageAvailable)
		{
			MessageQueue.addForce(IncomingMessage);
			IncomingMessageAvailable = false;
		}
		else if (!MessageQueue.isEmpty())
		{
			if (!MessageQueue.pull(CurrentMessage))
			{
				//Something must have gone wrong.
				OnMessageContentError();
			}

			if (!ProcessMessageInternal())
			{
				//Unrecognized message.
				OnMessageContentError();
			}
		}
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		else if (MessageErrorReportNeedsUpdating)
		{
			UpdateMessageErrorsReport();
		}
#endif
		else
		{
			disable();
			return false;
		}

		enable();
		return true;
	}

private:
	bool PrepareBaseMessages()
	{
		bool Success = true;
		//Id.
		IdMessage.Clear();
		IdMessage.SetHeader(I2C_SLAVE_BASE_HEADER_DEVICE_ID);
		Success &= IdMessage.Append32BitPayload(GetDeviceId());

		//Serial.
		SerialMessage.Clear();
		SerialMessage.SetHeader(I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL);
		Success &= IdMessage.Append32BitPayload(GetSerial());

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		//Message errors.
		MessageErrorsOverflowMessage.Clear();
		MessageErrorsOverflowMessage.SetHeader(I2C_SLAVE_BASE_HEADER_MESSAGE_OVERFLOWS);
		Success &= MessageErrorsOverflowMessage.Append32BitPayload(0);

		MessageErrorsBadSizeMessage.Clear();
		MessageErrorsBadSizeMessage.SetHeader(I2C_SLAVE_BASE_HEADER_MESSAGE_BAD_SIZE);
		Success &= MessageErrorsBadSizeMessage.Append32BitPayload(0);

		MessageErrorsContentMessage.Clear();
		MessageErrorsContentMessage.SetHeader(I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_CONTENT);
		Success &= MessageErrorsContentMessage.Append32BitPayload(0);


		if (Success)
		{
			//Updated with real values.
			UpdateMessageErrorsReport();
		}
#endif

#ifdef I2C_SLAVE_HEALTH_ENABLE
		HealthReportMessage.Clear();
		HealthReportMessage.SetHeader(I2C_SLAVE_BASE_HEADER_HEALTH_TEMPERATURE);
		Success &= HealthReportMessage.Append8BitPayload(0);

		VoltageMessage.Clear();
		VoltageMessage.SetHeader(I2C_SLAVE_BASE_HEADER_HEALTH_VOLTAGE);
		Success &= VoltageMessage.Append16BitPayload(0);

		TemperatureMessage.Clear();
		TemperatureMessage.SetHeader(I2C_SLAVE_BASE_HEADER_HEALTH_TEMPERATURE);
		Success &= TemperatureMessage.Append16BitPayload(0);
#endif

		return Success;
	}

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
	void UpdateMessageErrorsReport()
	{
		MessageErrorReportNeedsUpdating = false;
		MessageErrorsOverflowMessage.Set32BitPayload(MessageOverflows);
		MessageErrorsBadSizeMessage.Set32BitPayload(MessageSizeErrors);
		MessageErrorsContentMessage.Set32BitPayload(MessageContentErrors);
	}
#endif

	bool ProcessMessageInternal()
	{
		switch (CurrentMessage.GetHeader())
		{
		case I2C_SLAVE_BASE_HEADER_DEVICE_ID:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetId"));
#endif
			OutgoingMessage = &IdMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetSerial"));
#endif
			OutgoingMessage = &SerialMessage;
			return true;

#ifdef I2C_SLAVE_DEVICE_RESET_ENABLE
		case I2C_SLAVE_BASE_HEADER_RESET_DEVICE:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("Reset device, bye bye!"));
#endif
			ResetDevice();
			//Never runs;
			return true;
#endif

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		case I2C_SLAVE_BASE_HEADER_MESSAGE_OVERFLOWS:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetMessageOverflows"));
#endif
			OutgoingMessage = &MessageErrorsOverflowMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_MESSAGE_BAD_SIZE:
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetMessageSizeErrors"));
#endif
			OutgoingMessage = &MessageErrorsBadSizeMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_CONTENT:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetMessageContentErrors"));
#endif
			OutgoingMessage = &MessageErrorsContentMessage;
			return true;
#endif 

#ifdef I2C_SLAVE_HEALTH_ENABLE
		case I2C_SLAVE_BASE_HEADER_HEALTH_REPORT:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetHealthReport"));
#endif
			OutgoingMessage = &HealthReportMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_HEALTH_VOLTAGE:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetHealthVoltage"));
#endif
			OutgoingMessage = &VoltageMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_HEALTH_TEMPERATURE:
			if (CurrentMessage.GetLength() != I2C_MESSAGE_LENGTH_HEADER_ONLY)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSLAVE
			DebugSerial.println(F("GetTemperature"));
#endif
			OutgoingMessage = &TemperatureMessage;
			return true;
#endif 
		default:
			if (CurrentMessage.GetHeader() < I2C_SLAVE_BASE_HEADER)
			{
				return ProcessMessage(&CurrentMessage);
			}
		}

#ifdef DEBUG_ABSTRACT_I2CSLAVE
		DebugSerial.println(F("Unable to process message"));
#endif

		return false;
	}
};
#endif

