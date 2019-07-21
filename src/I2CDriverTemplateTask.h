// I2CDriverTemplateTask.h
#ifdef I2C_DRIVER_ASYNC_ENABLE
#ifndef _I2CDRIVERTEMPLATETASK_h
#define _I2CDRIVERTEMPLATETASK_h



#include <I2CDriverTemplate.h>
#include <Extras\DelayedCallback.h>
#include <RingBufCPP.h>

#define I2C_DRIVER_TEMPLATE_TASK_MIN_MILLIS_BETWEEN_MESSAGE 1
#define I2C_DRIVER_TEMPLATE_TASK_QUEUE_SIZE					10


template<typename WireClass,
	const uint8_t DeviceAddress,
	const uint8_t MessageMaxSize = I2C_MESSAGE_RECEIVER_MESSAGE_LENGTH_MIN,
	const uint8_t OutQueueSize = I2C_DRIVER_TEMPLATE_TASK_QUEUE_SIZE,
	const uint8_t MinDelayMillis = I2C_DRIVER_TEMPLATE_TASK_MIN_MILLIS_BETWEEN_MESSAGE>
class I2CDriverTemplateTask : public I2CDriverTemplate<WireClass, DeviceAddress, MessageMaxSize>
{
	RingBufCPP<TemplateMessageI2C<MessageMaxSize>, OutQueueSize> OutMessageQueue;

	uint32_t LastSentMillis = 0;

	DelayedCallback CallbackHandler;

protected:
	virtual bool OnSetup(WireClass* i2cInstance)
	{
		return true;
	}

	//Only returns true if message was written.
	bool WriteCurrentMessageAsync()
	{
		if (millis() - LastSentMillis >= MinDelayMillis)
		{
			LastSentMillis = millis();
			return WriteCurrentMessage();
		}
		else
		{
			OutMessageQueue.addForce(OutgoingMessage);
			CallbackHandler.RequestCallback(MinDelayMillis);
		}

		return false;
	}

	//TODO: 
	/*bool GetResponseAsync()
	{

	}*/

public:
	I2CDriverTemplateTask(Scheduler* scheduler) : I2CDriverTemplate<WireClass, DeviceAddress, MessageMaxSize>()
		, CallbackHandler(scheduler)
	{
		MethodSlot<I2CDriverTemplateTask, const uint8_t> memFunSlot(this, &I2CDriverTemplateTask::Callback);

		CallbackHandler.AttachCallback(memFunSlot);		
	}

	void Callback(const uint8_t param)
	{
		if (millis() - LastSentMillis >= MinDelayMillis)
		{
			if (OutMessageQueue.pull(OutgoingMessage))
			{
				WriteCurrentMessageAsync();
			}
		}
		else
		{
			CallbackHandler.RequestCallback(MinDelayMillis);
		}
	}
};
#endif
#endif