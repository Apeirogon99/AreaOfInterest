#include "ServerMessageHandler.h"
#include <iostream>

using namespace std;

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data)
{
}

void Handle_PKT_C2S_PATH_FINDING(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_PATH_FINDING* Protocol)
{
	const uint32_t timestamp = Protocol->TimeStamp;
	const GridPoint& destGridPoint = Protocol->DestGridPoint;

	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& destNode = map->mGrid[destGridPoint.Y][destGridPoint.X];

	if (destNode.get() == nullptr)
	{
		return;
	}

	auto sessionIter = World->mSessionToEntity.find(Session->GetSessionId());
	if (sessionIter == World->mSessionToEntity.end())
	{
		return;
	}

	auto entityIter = World->mEntitys.find(sessionIter->second);
	if (entityIter == World->mEntitys.end())
	{
		return;
	}

	// 경로 찾기
	{
		uint32_t time = static_cast<uint32_t>(gTimeManager.GetServerTime());
		World->PushTask(time, World, &World::PathFind, Session->GetSessionId(), destGridPoint.X, destGridPoint.Y);
	}
}

void Handle_PKT_C2S_ENTITY_INFO(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_ENTITY_INFO* Protocol)
{

	uint8_t nextInfoNumber = Protocol->InfoNumber + 1;
	if (nextInfoNumber >= EEntityInfoPriority::ENTITY_INFO_MAX)
	{
		// 보내야 하는 데이터를 전부 보냄
		return;
	}

#if USE_AOI
	// 100ms 딜레이하여 전송하기
	{
		uint32_t time = static_cast<uint32_t>(gTimeManager.GetServerTime());
		World->PushTask(time + 100, World, &World::NextEntityInfo, Session->GetSessionId(), nextInfoNumber, Protocol->ObjectId);
	}
#else
	// 즉시 전송하기
	{
		uint32_t time = static_cast<uint32_t>(gTimeManager.GetServerTime());
		World->PushTask(time, World, &World::NextEntityInfo, Session->GetSessionId(), nextInfoNumber, Protocol->ObjectId);
	}
#endif

}
