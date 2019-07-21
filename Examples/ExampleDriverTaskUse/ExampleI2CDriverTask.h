// ExampleI2CDriverTask.h

#if !defined(_EXAMPLE_I2C_DRIVERTASK_h) && !defined(__AVR_ATtiny85__)
#define _EXAMPLE_I2C_DRIVERTASK_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include <Arduino.h>
#include <extras\ExampleI2CDriver.h>
#include <DelayedCallback.h>

#define SERVOS_SLAVE_TASK_MAX_MESSAGE_QUEUE_SIZE	10		
#define SERVOS_SLAVE_TASK_MIN_PERIOD_BETWEEN_MILLS	(ceil(I2C_SERVOS_SLAVE_API_MIN_MICROS_BETWEEN_MESSAGE/1000))


template<const uint8_t OutQueueSize = SERVOS_SLAVE_TASK_MAX_MESSAGE_QUEUE_SIZE>
class ExampleI2CDriverTask : public ExampleI2CDriver
{
	RingBufCPP<TemplateMessageI2C<>, OutQueueSize> OutMessageQueue;

	uint32_t LastSentMillis = 0;

	DelayedCallback CallbackHandler;


private:
	bool WriteCurrentMessage()
	{
		if (millis() - LastSentMillis >= SERVOS_SLAVE_TASK_MIN_PERIOD_BETWEEN_MILLS)
		{
			LastSentMillis = millis();
			return I2CServosDriver::WriteCurrentMessage();
		}
		else
		{
			OutMessageQueue.addForce(OutgoingMessage);
			CallbackHandler.RequestCallback(SERVOS_SLAVE_TASK_MIN_PERIOD_BETWEEN_MILLS);
		}

		return false;
	}

public:
	ExampleI2CDriverTask(Scheduler* scheduler) : ExampleI2CDriver()
		, CallbackHandler(scheduler)
	{

	}

	bool Setup(WireBase* i2cInstance)
	{
		if (I2CServosDriver::Setup(i2cInstance))
		{
			MethodSlot<I2CServosDriverTask, const uint8_t> memFunSlot(this, &I2CServosDriverTask::Callback);

			CallbackHandler.AttachCallback(memFunSlot);

			return true;
		}
		return false;
	}

	bool OnEnable()
	{
		return true;
	}

	void Callback(const uint8_t param)
	{
		if (millis() - LastSentMillis >= SERVOS_SLAVE_TASK_MIN_PERIOD_BETWEEN_MILLS)
		{
			if (OutMessageQueue.pull(OutgoingMessage))
			{
				WriteCurrentMessage();
			}
		}
		else
		{
			CallbackHandler.RequestCallback(SERVOS_SLAVE_TASK_MIN_PERIOD_BETWEEN_MILLS);
		}
	}
};
#endif