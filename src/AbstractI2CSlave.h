// AbstractI2CSlave.h

#if !defined(_ABSTRACTI2CSLAVE_h) && defined(__AVR__)
#define _ABSTRACTI2CSLAVE_h

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

template <const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN, const uint8_t ReceiverQueueDepth = I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH>
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
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsOverflowMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsBadSizeMessage;
	TemplateMessageI2C<I2C_MESSAGE_LENGTH_32BIT_X1> MessageErrorsContentMessage;
	///

	///Error and Status for this session.
	volatile uint32_t MessageOverflows = 0;
	volatile uint32_t MessageSizeErrors = 0;
	volatile uint32_t MessageContentErrors = 0;
	volatile bool MessageErrorReportNeedsUpdating = false;
	///
#endif

	///Slave info messages.
	TemplateMessageI2C<MessageMaxSize> IdMessage;
	TemplateMessageI2C<MessageMaxSize> SerialMessage;
	///

protected:
	///I2C Read output message.
	TemplateMessageI2C<MessageMaxSize>* OutgoingMessage = nullptr;

protected:
	virtual bool ProcessMessage(TemplateMessageI2C<MessageMaxSize>* currentMessage) {}
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

public:
#ifdef I2C_SLAVE_USE_TASK_SCHEDULER
	AbstractI2CSlaveTask(Scheduler* scheduler) : I2CInterruptTask(scheduler)
#else
	AbstractI2CSlaveTask() : I2CInterruptTask()
#endif	
	{
		I2CHandler = this;
	}

	bool Setup(const uint8_t deviceAddress)
	{
		if (I2CHandler != nullptr &&
			deviceAddress > I2C_ADDRESS_MIN_VALUE
			&& deviceAddress < I2C_ADDRESS_MAX_VALUE)
		{
			///Overzealous I2C Setup.
			pinMode(PIN_WIRE_SCL, INPUT);
			pinMode(PIN_WIRE_SDA, INPUT);
			delay(1);
			Wire.flush();
			delay(1);
			///

			Wire.begin(deviceAddress); //Join i2c bus with address.
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

				return true;
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

	inline void OnReceive(int length)
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
			if (!IncomingMessage.Write(Wire.read()))
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
		if (OutgoingMessage != nullptr)
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
			enable();
		}
		else if (!MessageQueue.isEmpty())
		{
			if (!MessageQueue.pull(CurrentMessage))
			{
				//Something must have gone wrong.

				return true;
			}

			if (!ProcessMessageInternal())
			{
				//Unrecognized message.
				OnMessageContentError();
			}
			enable();

			return true;
		}
#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		else if (MessageErrorReportNeedsUpdating)
		{
			UpdateMessageErrorsReport();
			enable();

			return true;
		}
#endif
		else
		{
			disable();
		}

		return false;
	}

private:
	bool PrepareBaseMessages()
	{
		bool Success = true;
		//Id.
		IdMessage.Clear();
		Success &= IdMessage.Write(I2C_SLAVE_BASE_HEADER_DEVICE_ID);
		Success &= IdMessage.Append32BitPayload(GetDeviceId());

		//Serial.
		SerialMessage.Clear();
		Success &= SerialMessage.Write(I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL);
		Success &= IdMessage.Append32BitPayload(GetSerial());

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		//Message errors.
		MessageErrorsOverflowMessage.Clear();
		Success &= MessageErrorsOverflowMessage.Write(I2C_SLAVE_BASE_HEADER_MESSAGE_OVERFLOWS);
		Success &= MessageErrorsOverflowMessage.Append32BitPayload(0);

		MessageErrorsBadSizeMessage.Clear();
		Success &= MessageErrorsBadSizeMessage.Write(I2C_SLAVE_BASE_HEADER_MESSAGE_BAD_SIZE);
		Success &= MessageErrorsBadSizeMessage.Append32BitPayload(0);

		MessageErrorsContentMessage.Clear();
		Success &= MessageErrorsContentMessage.Write(I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_CONTENT);
		Success &= MessageErrorsContentMessage.Append32BitPayload(0);


		if (Success)
		{
			//Updated with real values.
			UpdateMessageErrorsReport();
		}
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
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSERVOS
			Serial.println(F("GetId"));
#endif
			OutgoingMessage = &IdMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL:
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSERVOS
			Serial.println(F("GetSerial"));
#endif
			OutgoingMessage = &SerialMessage;
			return true;

#ifdef I2C_SLAVE_COMMS_ERRORS_ENABLE
		case I2C_SLAVE_BASE_HEADER_MESSAGE_OVERFLOWS:
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSERVOS
			Serial.println(F("GetMessageOverflows"));
#endif
			OutgoingMessage = &MessageErrorsOverflowMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_MESSAGE_BAD_SIZE:
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSERVOS
			Serial.println(F("GetMessageSizeErrors"));
#endif
			OutgoingMessage = &MessageErrorsBadSizeMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_CONTENT:
			if (CurrentMessage.GetLength() != 1)
			{
				OnMessageSizeError();
				return true;
			}
#ifdef DEBUG_ABSTRACT_I2CSERVOS
			Serial.println(F("GetMessageContentErrors"));
#endif
			OutgoingMessage = &MessageErrorsContentMessage;
			return true;
#endif 
		default:
			if (CurrentMessage.GetHeader() < I2C_SLAVE_BASE_HEADER)
			{
				return ProcessMessage(&CurrentMessage);
			}
		}

#ifdef DEBUG_ABSTRACT_I2CSERVOS
		Serial.println(F("Unable to process message"));
#endif

		return false;
	}
};
#endif

