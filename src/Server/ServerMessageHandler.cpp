#include "ServerMessageHandler.h"
#include <iostream>

using namespace std;

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data)
{
}

void Handle_PKT_C2S_PATH_FINDING(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_PATH_FINDING* Protocol)
{
	const uint32_t timestamp = Protocol->TimeStamp;
	const int destGridX = Protocol->DestGridX;
	const int destGridY = Protocol->DestGridY;

	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& destNode = map->mGrid[destGridY][destGridX];

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
	const std::unique_ptr<Entity>& entity = entityIter->second;

	// 경로 찾기
	{
		World->PushTask(0, World, &World::PathFind, entity.get(), destNode.get());
	}
}

void Handle_PKT_C2S_TOGGLE_WALKABLE(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_TOGGLE_WALKABLE* Protocol)
{
	const int gridX = Protocol->GridX;
	const int gridY = Protocol->GridY;

	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& node =  map->mGrid[gridY][gridX];

	if (gridX < 0 || gridY < 0 || gridX >= map->mGridSizeX || gridY >= map->mGridSizeY)
	{
		return;
	}

	// 블록 변경
	{
		World->PushTask(0, World, &World::ToggleWalkeable, gridX, gridY);
	}

	// 경로 재탐색 (아직 테스트 중)
	{
		World->PushTask(1, World, &World::PathRefind);
	}
}
