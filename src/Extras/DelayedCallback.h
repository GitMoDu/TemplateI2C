// DelayedCallback.h

#ifndef _DELAYED_CALLBACK_h
#define _DELAYED_CALLBACK_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>
#include <Callback.h>

class DelayedCallback : Task
{
private:
	Signal<const uint8_t> CallbackEvent;

public:
	DelayedCallback(Scheduler* scheduler)
		: Task(0, TASK_FOREVER, scheduler, false)
	{
	}

	void RequestCallback(const uint8_t delayMillis)
	{
		enable();
		Task::delay(delayMillis);
	}

	void AttachCallback(const Slot<const uint8_t>& slot)
	{
		CallbackEvent.attach(slot);
	}

	bool OnEnable()
	{
		forceNextIteration();
		return true;
	}

	void OnDisable()
	{
	}

	bool Callback()
	{
		disable();
		CallbackEvent.fire(0);

		return true;
	}
};
#endif