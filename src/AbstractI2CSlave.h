// AbstractI2CSlave.h

#ifndef _ABSTRACTI2CSLAVE_h
#define _ABSTRACTI2CSLAVE_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>
#include <RingBufCPP.h>
#include <Wire.h>
#include <IMessageI2C.h>

#define I2C_BUFFER_SIZE								32
#define TWI_RX_BUFFER_SIZE							( I2C_BUFFER_SIZE )
#define TWI_TX_BUFFER_SIZE							( I2C_BUFFER_SIZE )

#define I2C_ADDRESS_MIN_VALUE						0x10
#define I2C_ADDRESS_MAX_VALUE						0xC0

#define I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH	10


class I2CInterruptTask : public Task
{
public:
	virtual void OnReceive(int length) {}
	virtual void OnRequest() {}

	I2CInterruptTask(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, true)
	{
	}
};

I2CInterruptTask* I2CHandler = nullptr;

void ReceiveEvent(int length)
{
	if (I2CHandler != nullptr)
	{
		I2CHandler->OnReceive(length);
	}
}

void RequestEvent()
{
	if (I2CHandler != nullptr)
	{
		I2CHandler->OnRequest();
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

	///I2C. //TODO: make #defined variant for AtTiny.
	TwoWire* I2CInstance = nullptr;
	///

	///Error and Status for this session.
	MessageClass MessageErrorsReportMessage;
	MessageClass IdMessage;
	MessageClass SerialMessage;
	///

	///Array helper
	ArrayToUint32 Helper32Bit;

protected:
	///I2C Read output message.
	MessageClass* OutgoingMessage = nullptr;

	///Error and Status for this session.
	volatile uint32_t MessageOverflows = 0;
	volatile uint32_t MessageSizeErrors = 0;
	volatile uint32_t MessageProcessingErrors = 0;
	volatile uint32_t QueueErrors = 0;
	volatile uint32_t LastI2CEventMillis = 0;

	volatile bool MessageErrorReportNeedsUpdating = false;
	///

protected:
	virtual bool ProcessMessage(MessageClass* currentMessage) {}
	virtual bool OnSetup() { return false; }
	virtual uint32_t GetDeviceId() { return 0; }
	virtual uint32_t GetSerial() { return 0; }
public:
	AbstractI2CSlaveTask(Scheduler* scheduler)
		: I2CInterruptTask(scheduler)
	{
		I2CHandler = this;
	}

	bool Setup(TwoWire* i2CInstance, const uint8_t deviceAddress)
	{
		if (deviceAddress > I2C_ADDRESS_MIN_VALUE
			&& deviceAddress < I2C_ADDRESS_MAX_VALUE)
		{
			I2CInstance = i2CInstance;

			if (I2CInstance != nullptr)
			{
				///Overzealous I2C Setup.
				pinMode(PIN_WIRE_SCL, INPUT);
				pinMode(PIN_WIRE_SDA, INPUT);
				delay(1);
				I2CInstance->flush();
				delay(1);
				///

				I2CInstance->begin(deviceAddress); //Join i2c bus with address.
				I2CInstance->onReceive(ReceiveEvent);
				I2CInstance->onRequest(RequestEvent);

				if (OnSetup())
				{
					MessageOverflows = 0;
					MessageSizeErrors = 0;
					MessageProcessingErrors = 0;
					QueueErrors = 0;

					PrepareBaseMessages();

					return true;
				}
			}
		}

		return false;
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
			MessageSizeErrors++;

			enableIfNot();

			return;
		}

		if (IncomingMessageAvailable)
		{
			//Cannot respond so quickly, hold your horses.
			//If this happens, we've skipped a message.
			MessageOverflows++;
			MessageErrorReportNeedsUpdating = true;

			enableIfNot();

			return;
		}

		IncomingMessage.Clear();
		//We copy to a second buffer, so we can process it in the main loop safely, instead of in the interrupt.
		while (length--)
		{
			if (!IncomingMessage.Write(I2CInstance->read()))
			{
				MessageSizeErrors++;
				MessageErrorReportNeedsUpdating = true;

				enableIfNot();

				return;
			}
		}

		IncomingMessageAvailable = true;
		enable();

		LastI2CEventMillis = millis;
	}

	void OnRequest()
	{
		if (OutgoingMessage != nullptr)
		{
			I2CInstance->write(OutgoingMessage->GetRaw(), (size_t)min(TWI_TX_BUFFER_SIZE, OutgoingMessage->GetLength()));
		}
		LastI2CEventMillis = millis;
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
				QueueErrors++;
				MessageErrorReportNeedsUpdating = true;

				return true;
			}

			if (ProcessMessageInternal())
			{
				//Unrecognized message.
				MessageProcessingErrors++;
				MessageErrorReportNeedsUpdating = true;
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
	inline void PrepareBaseMessages()
	{
		//Id.
		IdMessage.Clear();
		IdMessage.Write(I2C_SLAVE_BASE_HEADER_DEVICE_ID);
		Helper32Bit.uint = GetDeviceId();
		IdMessage.Write(Helper32Bit.array, 4);

		//Serial.
		SerialMessage.Clear();
		SerialMessage.Write(I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL);
		Helper32Bit.uint = GetSerial();
		SerialMessage.Write(Helper32Bit.array, 4);

		//Message errors Report.
		MessageErrorsReportMessage.Clear();
		MessageErrorsReportMessage.Write(I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_REPORT);
		//Fill in with zeros until the message size = 1 + 16 bytes.
		Helper32Bit.uint = 0;
		MessageErrorsReportMessage.Write(Helper32Bit.array, 4);
		MessageErrorsReportMessage.Write(Helper32Bit.array, 4);
		MessageErrorsReportMessage.Write(Helper32Bit.array, 4);
		MessageErrorsReportMessage.Write(Helper32Bit.array, 4);
		//Updated with real values.
		UpdateMessageErrorsReport();
	}

	void UpdateMessageErrorsReport()
	{
		MessageErrorReportNeedsUpdating = false;

		Helper32Bit.uint = MessageOverflows;
		MessageErrorsReportMessage.GetRaw()[1] = Helper32Bit.array[0];
		MessageErrorsReportMessage.GetRaw()[2] = Helper32Bit.array[1];
		MessageErrorsReportMessage.GetRaw()[3] = Helper32Bit.array[2];
		MessageErrorsReportMessage.GetRaw()[4] = Helper32Bit.array[3];

		Helper32Bit.uint = MessageSizeErrors;
		MessageErrorsReportMessage.GetRaw()[5] = Helper32Bit.array[0];
		MessageErrorsReportMessage.GetRaw()[6] = Helper32Bit.array[1];
		MessageErrorsReportMessage.GetRaw()[7] = Helper32Bit.array[2];
		MessageErrorsReportMessage.GetRaw()[8] = Helper32Bit.array[3];

		Helper32Bit.uint = MessageProcessingErrors;
		MessageErrorsReportMessage.GetRaw()[9] = Helper32Bit.array[0];
		MessageErrorsReportMessage.GetRaw()[10] = Helper32Bit.array[1];
		MessageErrorsReportMessage.GetRaw()[11] = Helper32Bit.array[2];
		MessageErrorsReportMessage.GetRaw()[12] = Helper32Bit.array[3];

		Helper32Bit.uint = QueueErrors;
		MessageErrorsReportMessage.GetRaw()[13] = Helper32Bit.array[0];
		MessageErrorsReportMessage.GetRaw()[14] = Helper32Bit.array[1];
		MessageErrorsReportMessage.GetRaw()[15] = Helper32Bit.array[2];
		MessageErrorsReportMessage.GetRaw()[16] = Helper32Bit.array[3];
	}

	bool ProcessMessageInternal()
	{
		switch (CurrentMessage.GetHeader())
		{
		case I2C_SLAVE_BASE_HEADER_DEVICE_ID:
			OutgoingMessage = &IdMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_DEVICE_SERIAL:
			OutgoingMessage = &SerialMessage;
			return true;
		case I2C_SLAVE_BASE_HEADER_MESSAGE_ERROR_REPORT:
			OutgoingMessage = &MessageErrorsReportMessage;
			return true;
		default:
			if(CurrentMessage.GetHeader() < I2C_SLAVE_BASE_HEADER)
			{
				return ProcessMessage(&CurrentMessage);
			}
		}

		return false;
	}
};
#endif

