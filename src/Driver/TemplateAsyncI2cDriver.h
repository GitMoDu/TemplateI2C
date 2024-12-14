// TemplateAsyncI2cDriver.h

#ifndef _TEMPLATE_ASYNC_I2C_DRIVER_h
#define _TEMPLATE_ASYNC_I2C_DRIVER_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "TemplateI2cDriver.h"
#include "AsyncRequester.h"

namespace TemplateI2c
{
	namespace AsyncDriver
	{
		template<const uint8_t address, const uint32_t id, uint16_t replyMinDelay = 25>
		class I2cDriver : public virtual AsyncI2cListener, public TemplateI2c::I2cDriver<address, id, replyMinDelay>
		{
		private:
			using Base = TemplateI2c::I2cDriver<address, id, replyMinDelay>;

		protected:
			using Base::SendMessage;
			using Base::Incoming;
			using Base::GetReplyDelay;

		private:
			AsyncRequester<address> Requester;

		public:
			I2cDriver(TS::Scheduler& scheduler, TwoWire& wire)
				: AsyncI2cListener()
				, Base(wire)
				, Requester(scheduler, wire, Incoming, this)
			{
			}

			const bool GetDeviceIdAsync()
			{
				return RequestResponse(Api::Requests::GetId::Header, Api::Requests::GetId::ReplySize, Api::Requests::GetId::ReplyDelay);
			}

		protected:
			const bool RequestResponse(const uint8_t header, uint8_t* payload, const uint8_t payloadSize, const uint8_t responseSize, const uint16_t delayMicros = 0)
			{
				if (Requester.CanRequest()
					&& SendMessage(header, payload, payloadSize))
				{
					return Requester.WaitForResponse(header, responseSize, GetReplyDelay(delayMicros));
				}

				return false;
			}

			const bool RequestResponse(const uint8_t header, const uint8_t responseSize, const uint16_t delayMicros = 0)
			{
				if (Requester.CanRequest()
					&& SendMessage(header))
				{
					return Requester.WaitForResponse(header, responseSize, GetReplyDelay(delayMicros));
				}

				return false;
			}

			/// <summary>
			/// Overrideable listener callbacks.
			/// </summary>
		public:
			virtual void OnI2cError(const ErrorEnum error) {}

			virtual void OnI2cReceived(const uint8_t requestHeader, const uint8_t size) {}
		};
	}
}

#endif