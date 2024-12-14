// AsyncRequester.h

#ifndef _ASYNC_I2C_REQUESTER_h
#define _ASYNC_I2C_REQUESTER_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

namespace TemplateI2c
{
	namespace AsyncDriver
	{
		enum class ErrorEnum
		{
			InvalidSize,
			ReadOverflow,
			RequestFail
		};

		struct AsyncI2cListener
		{
			virtual void OnI2cReceived(const uint8_t requestHeader, const uint8_t size) {}
			virtual void OnI2cError(const ErrorEnum error) {}
		};

		template<const uint8_t address>
		class AsyncRequester : private TS::Task
		{
		private:
			enum class StateEnum
			{
				NotRequesting,
				WaitingForRequestDelay,
				ReadingReplyData,
				RecoveringFromError
			};

		private:
			TwoWire& WireInstance;
			uint8_t* InBuffer;

			AsyncI2cListener* Listener;

		private:
			uint32_t RequestStart = 0;
			uint16_t RequestDelay = 0;
			uint16_t ResponseSize = 0;
			uint8_t RequestHeader = 0;
			StateEnum State = StateEnum::NotRequesting;

		private:
			uint_fast8_t InSize = 0;

		public:
			AsyncRequester(TS::Scheduler& scheduler,
				TwoWire& wire,
				uint8_t* inBuffer,
				AsyncI2cListener* receiveListener)
				: TS::Task(TASK_IMMEDIATE, TASK_FOREVER, &scheduler, false)
				, WireInstance(wire)
				, InBuffer(inBuffer)
				, Listener(receiveListener)
			{
			}

			const bool CanRequest() const
			{
				return State == StateEnum::NotRequesting;
			}

			const bool Setup() const
			{
				return InBuffer != nullptr && Listener != nullptr;
			}

			const bool WaitForResponse(const uint8_t header, const uint8_t responseSize, const uint16_t delayMicros)
			{
				if (CanRequest())
				{
					State = StateEnum::WaitingForRequestDelay;
					TS::Task::enable();
					RequestStart = micros();
					ResponseSize = responseSize;
					RequestHeader = header;
					RequestDelay = delayMicros;

					return true;
				}

				return false;
			}

			virtual bool Callback() final
			{
				switch (State)
				{
				case StateEnum::WaitingForRequestDelay:
					if ((micros() - RequestStart) >= RequestDelay)
					{
						if (Wire.requestFrom(address, ResponseSize))
						{
							InSize = GetResponse(ResponseSize);
							State = StateEnum::ReadingReplyData;
						}
						else
						{
							State = StateEnum::NotRequesting;
							Listener->OnI2cError(ErrorEnum::RequestFail);
						}
					}
					break;
				case StateEnum::ReadingReplyData:					
					if (InSize >= ResponseSize)
					{
						Listener->OnI2cReceived(RequestHeader, ResponseSize);
						if (InSize > ResponseSize || Wire.available())
						{
							State = StateEnum::RecoveringFromError;
							Listener->OnI2cError(ErrorEnum::ReadOverflow);
						}
						else
						{
							InSize = 0;
							State = StateEnum::NotRequesting;
						}
					}
					else
					{
						State = StateEnum::RecoveringFromError;
						Listener->OnI2cError(ErrorEnum::InvalidSize);
					}
					break;
				case StateEnum::RecoveringFromError:
					InSize = 0;
					State = StateEnum::NotRequesting;
					while (Wire.available())
					{
						Wire.read();
					}
					break;
				case StateEnum::NotRequesting:
				default:
					TS::Task::disable();
					break;
				}

				return true;
			}

		private:
			const uint8_t GetResponse(const uint8_t responseSize)
			{
				uint_fast8_t size = 0;
				while (Wire.available()
					&& size < responseSize)
				{
					InBuffer[size++] = Wire.read();
				}

				return size;
			}
		};

	}
}
#endif