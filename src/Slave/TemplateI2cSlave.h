// TemplateI2cSlave.h

#ifndef _TEMPLATE_I2C_SLAVE_h
#define _TEMPLATE_I2C_SLAVE_h

#include "TemplateI2c.h"
#include "../Platform/Platform.h"
#include <Wire.h>

namespace TemplateI2c
{
	class AbstractSlave
	{
	protected:
		uint8_t Incoming[MessageMaxSize]{};

	protected:
		TwoWire& WireInstance;

	public:
		AbstractSlave(TwoWire& wire)
			: WireInstance(wire)
		{
		}

	protected:
		const bool SetupI2c(void (*onReceive)(int length), void (*onRequest)(), const uint8_t address)
		{
			if (address >= I2cAddressMin
				&& address <= I2cAddressMax
				&& onReceive != nullptr)
			{
				Wire.begin(address);

				Wire.onReceive(onReceive);
				if (onRequest != nullptr)
				{
					Wire.onRequest(onRequest);
				}


				return true;
			}

			return false;
		}
	};

	template<uint8_t address, uint32_t id>
	class I2cSlave : AbstractSlave
	{
	private:
		const uint8_t IdMessage[Api::Requests::GetId::ReplySize]
		{
			(uint8_t)(id >> 24), (uint8_t)(id >> 16), (uint8_t)(id >> 8), (uint8_t)(id)
		};

	protected:
		using AbstractSlave::WireInstance;
		using AbstractSlave::Incoming;

	private:
		// Used for I2C read as data source. Can be dinamically set but never nullptr.
		const uint8_t* Outgoing;
		volatile uint8_t OutgoingSize = 0;

	public:
		I2cSlave(TwoWire& wire)
			: AbstractSlave(wire)
			, Outgoing((uint8_t*)IdMessage)
		{
		}

		virtual const bool Setup(void (*onReceive)(int length), void (*onRequest)())
		{
			return AbstractSlave::SetupI2c(onReceive, onRequest, address);
		}

		void OnRequestInterrupt()
		{
			// Outgoing is always valid and ready to send.
			WireInstance.write(Outgoing, OutgoingSize);
		}

		void OnSystemMessageReceived(const uint8_t header)
		{
			switch (header)
			{
			case Api::Requests::GetId::Header:
				SetOutMessage((uint8_t*)IdMessage, Api::Requests::GetId::ReplySize);
				break;
			case Api::Requests::Reset::Header:
				ResetDevice();
				break;
			default:
				break;
			}
		}

	protected:
		const uint8_t ReadIncoming(const uint8_t size)
		{
			uint_fast8_t i = 0;
			for (i = 0; i < size; i++)
			{
				if (WireInstance.available())
				{
					Incoming[i] = WireInstance.read();
				}
				else
				{
					break;
				}
			}

			return i;
		}

		void SetOutMessage(const uint8_t* message, const uint8_t size)
		{
			Outgoing = message;
			OutgoingSize = size;
		}

		void ResetDevice()
		{
			Platform::ResetCall();
		}
	};
}
#endif