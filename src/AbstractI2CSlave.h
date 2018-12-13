// AbstractI2CSlave.h

#ifndef _ABSTRACTI2CSLAVE_h
#define _ABSTRACTI2CSLAVE_h

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>
#include <RingBufCPP.h>
#include <Wire.h>
#include <IMessageI2C.h>

#define I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH	32


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

template <class MessageClass : public IMessageI2C, const uint8_t ReceiverQueueDepth = I2C_MESSAGE_RECEIVER_QUEUE_DEFAULT_DEPTH>
class AbstractI2CSlaveTask : public I2CInterruptTask
{
private:
	RingBufCPP<MessageClass, ReceiverQueueDepth> MessageQueue;

	volatile bool IncomingMessageAvailable = false;
	MessageClass IncomingMessage;
	MessageClass* OutgoingMessage = nullptr;

protected:
	MessageClass CurrentMessage;

	uint8_t CurrentHeader;
	uint8_t CurrentTargetChannel;

public:
	AbstractI2CSlaveTask(Scheduler* scheduler)
		: I2CInterruptTask(scheduler)
	{
		I2CHandler = this;
	}

	void ReceiveEvent(int length)
	{
		// Sanity-check.
		if (length < 1 ||
			length > TWI_RX_BUFFER_SIZE)
		{
			return;
		}

		IncomingMessage.Clear();
		////We copy to a second buffer, so we can process it in the main loop safely, instead of in the interrupt.
		while (length--)
		{
			if (!IncomingMessage.Write(Wire.read()))
			{
				break;
			}
		}

		IncomingMessageAvailable = true;
		enable();
	}

	void RequestEvent()
	{
		if (OutgoingMessage != nullptr)
		{
			Wire.write((uint8_t *)OutgoingMessage->GetRaw(), (size_t)OutgoingMessage->GetLength());
		}
	}

	bool OnEnable()
	{
		return true;
	}

	bool Callback()
	{
		if (IncomingMessageAvailable)
		{
			MessageQueue.addForce(*IncomingMessage);
			IncomingMessageAvailable = false;
			forceNextIteration();
		}
		else if (!MessageQueue.isEmpty())
		{
			if (!MessageQueue.pull(CurrentMessage))
			{
				return true;
			}

			ProcessCurrentMessage();

			forceNextIteration();

			return true;
		}
		else
		{
			disable();
		}

		return false;
	}

protected:
	virtual void ProcessCurrentMessage() {}

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

#endif

