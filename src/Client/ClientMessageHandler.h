#pragma once
#include "Common/Protocol/Message.h"
#include "Common/Protocol/Protocol.h"
#include "World.h"

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data);
void Handle_PKT_S2C_SPAWN_ENTITY(const std::unique_ptr<World>& World, const S2C_SPAWN_ENTITY* Protocol);
void Handle_PKT_S2C_APPEAR_ENTITY(const std::unique_ptr<World>& World, const S2C_APPEAR_ENTITY* Protocol);
void Handle_PKT_S2C_DISAPPEAR_ENTITY(const std::unique_ptr<World>& World, const S2C_DISAPPEAR_ENTITY* Protocol);
void Handle_PKT_S2C_ENTITY_INFO(const std::unique_ptr<World>& World, const S2C_ENTITY_INFO* Protocol);
void Handle_PKT_S2C_PATH_FINDING(const std::unique_ptr<World>& World, const S2C_PATH_FINDING* Protocol);
void Handle_PKT_S2C_POSITION_SYNC(const std::unique_ptr<World>& World, const S2C_POSITION* Protocol);

class ClientMessageHandler
{
public:
	static void ProcessMessage(const std::unique_ptr<World>& World, std::unique_ptr<Message> Message)
	{
		uint16_t id = Message->GetId();
		uint16_t size = Message->GetSize();
		std::vector<BYTE> data = Message->GetData();

		switch (static_cast<EMessageId>(id))
		{
		case EMessageId::PKT_S2C_SPAWN_ENTITY:
			Handle_PKT_S2C_SPAWN_ENTITY(World, MessageSerializer::Deserialize<S2C_SPAWN_ENTITY>(data, size));
			break;
		case EMessageId::PKT_S2C_APPEAR_ENTITY:
			Handle_PKT_S2C_APPEAR_ENTITY(World, MessageSerializer::Deserialize<S2C_APPEAR_ENTITY>(data, size));
			break;
		case EMessageId::PKT_S2C_DISAPPEAR_ENTITY:
			Handle_PKT_S2C_DISAPPEAR_ENTITY(World, MessageSerializer::Deserialize<S2C_DISAPPEAR_ENTITY>(data, size));
			break;
		case EMessageId::PKT_S2C_ENTITY_INFO:
			Handle_PKT_S2C_ENTITY_INFO(World, MessageSerializer::Deserialize<S2C_ENTITY_INFO>(data, size));
			break;
		case EMessageId::PKT_S2C_PATH_FINDING:
			Handle_PKT_S2C_PATH_FINDING(World, MessageSerializer::Deserialize<S2C_PATH_FINDING>(data, size));
			break;
		case EMessageId::PKT_S2C_POSITION_SYNC:
			Handle_PKT_S2C_POSITION_SYNC(World, MessageSerializer::Deserialize<S2C_POSITION>(data, size));
			break;
		default:
			Handle_INVALID(id, size, data);
			break;
		}
	}
};

