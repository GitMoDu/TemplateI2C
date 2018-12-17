// AbstractI2CSlave.h

#ifndef _ABSTRACTI2CSLAVE_h
#define _ABSTRACTI2CSLAVE_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>
#include <RingBufCPP.h>
#include <TemplateMessageI2C.h>
#include <I2CSlaveConstants.h>

#include <Wire.h>


class I2CInterruptTask : public Task
{
protected:
	volatile uint32_t LastI2CEventMillis = 0;

public:
	virtual void OnReceive(int length) {}
	virtual void OnRequest() {}

	I2CInterruptTask(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, true)
	{
	}

	void TimeStampI2CEvent()
	{
		LastI2CEventMillis = millis();
	}
};

I2CInterruptTask* I2CHandler = nullptr;

void ReceiveEvent(int length)
{
	if (I2CHandler != nullptr)
	{
		I2CHandler->OnReceive(length);
		I2CHandler->TimeStampI2CEvent();
	}
}

void RequestEvent()
{
	if (I2CHandler != nullptr)
	{
		I2CHandler->OnRequest();
		I2CHandler->TimeStampI2CEvent();
	}
}

template <typename MessageClass, const uint8_t ReceiverQueueDepth = I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH>
class AbstractI2CSlaveTask : public I2CInterruptTask
{
private:
	///Message queue.
	volatile bool IncomingMessageAvailable = false;
	RingBufCPP<MessageClass, ReceiverQueueDepth> MessageQueue;
	MessageClass IncomingMessage;
	MessageClass CurrentMessage;
	///

	///Error and Status for this session.
	MessageClass MessageErrorsOverflowMessage;
	MessageClass MessageErrorsBadSizeMessage;
	MessageClass MessageErrorsContentMessage;
	MessageClass IdMessage;
	MessageClass SerialMessage;
	///

	///Error and Status for this session.
	volatile uint32_t MessageOverflows = 0;
	volatile uint32_t MessageSizeErrors = 0;
	volatile uint32_t MessageContentErrors = 0;

	volatile bool MessageErrorReportNeedsUpdating = false;
	///

protected:
	///I2C Read output message.
	MessageClass* OutgoingMessage = nullptr;

protected:
	virtual bool ProcessMessage(MessageClass* currentMessage) {}
	virtual bool OnSetup() { return true; }
	virtual uint32_t GetDeviceId() { return 0; }
	virtual uint32_t GetSerial() { return 0; }

protected:
	void OnMessageOverflowError()
	{
		MessageOverflows++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
	}

	void OnMessageSizeError()
	{
		MessageSizeErrors++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
	}

	void OnMessageContentError()
	{
		MessageContentErrors++;
		MessageErrorReportNeedsUpdating = true;
		enableIfNot();
	}

public:
	AbstractI2CSlaveTask(Scheduler* scheduler)
		: I2CInterruptTask(scheduler)
	{
		I2CHandler = this;
	}

	bool Setup(const uint8_t deviceAddress)
	{
		if (deviceAddress > I2C_ADDRESS_MIN_VALUE
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

			MessageOverflows = 0;
			MessageSizeErrors = 0;
			MessageContentErrors = 0;

			if (PrepareBaseMessages())
			{
				LastI2CEventMillis = 0;

				return true;
			}
		}

		return false;
	}

	void AddForceMessage(MessageClass* message)
	{
		MessageQueue.addForce(*message);
		enable();
	}

	void AddMessage(MessageClass* message)
	{
		IncomingMessage = *message;
		IncomingMessageAvailable = true;
		enable();
	}

	bool OnEnable()
	{
		return true;
	}

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
			if (!IncomingMessage.Write(Wire.read()))
			{
				OnMessageSizeError();

				return;
			}
		}

		IncomingMessageAvailable = true;
		enable();
	}

	void OnRequest()
	{
		if (OutgoingMessage != nullptr)
		{
			Wire.write(OutgoingMessage->GetRaw(), (size_t)min(TWI_TX_BUFFER_SIZE, OutgoingMessage->GetLength()));
		}
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
		else if (MessageErrorReportNeedsUpdating)
		{
			UpdateMessageErrorsReport();
			enable();

			return true;
		}
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

		return Success;
	}

	void UpdateMessageErrorsReport()
	{
		MessageErrorReportNeedsUpdating = false;
		MessageErrorsOverflowMessage.Set32BitPayload(MessageOverflows);
		MessageErrorsBadSizeMessage.Set32BitPayload(MessageSizeErrors);
		MessageErrorsContentMessage.Set32BitPayload(MessageContentErrors);
	}

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
		default:
			if (CurrentMessage.GetHeader() < I2C_SLAVE_BASE_HEADER)
			{
				return ProcessMessage(&CurrentMessage);
			}
		}

		return false;
	}
};
#endif

