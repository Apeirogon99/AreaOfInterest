#pragma once
#include "Common/Protocol/Message.h"
#include "Common/Protocol/Protocol.h"

#include "World.h"
#include "Session.h"

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data);
void Handle_PKT_C2S_PATH_FINDING(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_PATH_FINDING* Protocol);

class ServerMessageHandler
{
public:
	static void ProcessMessage(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, std::unique_ptr<Message> Message)
	{
		uint16_t id = Message->GetId();
		uint16_t size = Message->GetSize();
		std::vector<BYTE> data = Message->GetData();

		switch (static_cast<EMessageId>(id))
		{
		case EMessageId::PKT_C2S_PATH_FINDING:
			Handle_PKT_C2S_PATH_FINDING(World, Session, MessageSerializer::Deserialize<C2S_PATH_FINDING>(data, size));
			break;
		default:
			Handle_INVALID(id, size, data);
			break;
		}
	}
};

