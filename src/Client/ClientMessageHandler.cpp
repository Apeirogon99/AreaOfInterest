#include "ClientMessageHandler.h"
#include "Common/Utils/Time.h"

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data)
{
}

void Handle_PKT_S2C_SPAWN_ENTITY(const std::unique_ptr<World>& World, const S2C_SPAWN_ENTITY* Protocol)
{
	const int objectId = Protocol->ObjectId;
	const int gridX = Protocol->GridX;
	const int gridY = Protocol->GridY;

	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& node = map->mGrid[gridY][gridX];

	if (World->mLocalEntityId == 0)
	{
		World->mLocalEntityId = objectId;
	}

	Vector2f spawnPosition = node->mPosition;
	World->mEntitys.insert({ objectId, std::make_unique<Entity>(objectId, spawnPosition) });
	World->mServerEntitys.insert({ objectId, std::make_unique<Entity>(objectId, spawnPosition) });
	printf("CreateEntity(%d) [%d][%d]\n", objectId, node->mGridY, node->mGridX);
}

void Handle_PKT_S2C_LOAD_MAP(const std::unique_ptr<World>& World, const S2C_LOAD_MAP* Protocol)
{
	/* 서버/클라이언트 동일 */
}

void Handle_PKT_S2C_PATH_FINDING(const std::unique_ptr<World>& World, const S2C_PATH_FINDING* Protocol)
{
	const int objectId = Protocol->ObjectId;

	auto clientEntityIter = World->mEntitys.find(Protocol->ObjectId);
	if (clientEntityIter == World->mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& clientEntity = clientEntityIter->second;

	auto serverEntityIter = World->mServerEntitys.find(Protocol->ObjectId);
	if (serverEntityIter == World->mServerEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& serverEntity = serverEntityIter->second;
	serverEntity->mPosition = { Protocol->EntityPositionX, Protocol->EntityPositionY };

	// 지연 시간만큼 예측 적용
	{
		long long serverTimestamp = Protocol->TimeStamp;
		long long clientTimestamp = Time::GetCurrentTimeMs();

		float delayTimestamp = static_cast<float>(clientTimestamp - serverTimestamp) / 1000.0f;
		serverEntity->MoveTowardsNextPath(delayTimestamp);
		clientEntity->MoveTowardsNextPath(delayTimestamp);

		printf("[방향 전환]\n");
		//printf("[Handle_PKT_S2C_PATH_FINDING]");
		//printf("Delay : %f\t", delayTimestamp);
	}

	// 서버와 클라이언트의 오차 확인 (디버그)
	{
		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		//printf("Distance : %f\n", distance);
	}

	// 클라이언트 보정
	{
		const float SNAP_THRESHOLD = 10.0f;		// 강제
		const float SMOOTH_THRESHOLD = 1.5f;	// 스무스

		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		if (distance > SNAP_THRESHOLD)
		{
			clientEntity->mPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = false;
			// 속도는 동일하니까 패스
		}
		else if (distance > SMOOTH_THRESHOLD && !clientEntity->mIsCorrection)
		{
			clientEntity->mCorrectionStartPosition = clientEntity->mPosition;
			clientEntity->mCorrectionEndPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = true;
			clientEntity->mCorrectionTime = 0.0f;
		}
	}

	std::list<Node*> serverPath;
	for (int index = 0; index < Protocol->PathCount; ++index)
	{
		Waypoint waypoint = Protocol->Path[index];
		serverPath.emplace_back(World->mMap->mGrid[waypoint.GridY][waypoint.GridX].get());
	}

	if (!CompareNodePositions(clientEntity->mPath, serverPath))
	{
		clientEntity->mPath.clear();
		clientEntity->mPath.swap(serverPath);
	}
}

void Handle_PKT_S2C_PATH_REFINDING(const std::unique_ptr<World>& World, const S2C_PATH_REFINDING* Protocol)
{
	const int objectId = Protocol->ObjectId;

	auto clientEntityIter = World->mEntitys.find(Protocol->ObjectId);
	if (clientEntityIter == World->mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& clientEntity = clientEntityIter->second;

	auto serverEntityIter = World->mServerEntitys.find(Protocol->ObjectId);
	if (serverEntityIter == World->mServerEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& serverEntity = serverEntityIter->second;
	//serverEntity->mPosition = { Protocol->EntityPositionX, Protocol->EntityPositionY };

	// 지연 시간만큼 예측 적용
	{
		long long serverTimestamp = Protocol->TimeStamp;
		long long clientTimestamp = Time::GetCurrentTimeMs();

		float delayTimestamp = static_cast<float>(clientTimestamp - serverTimestamp) / 1000.0f;
		serverEntity->MoveTowardsNextPath(delayTimestamp);
		clientEntity->MoveTowardsNextPath(delayTimestamp);

		//printf("[Handle_PKT_S2C_PATH_FINDING]");
		//printf("Delay : %f\t", delayTimestamp);
	}

	// 서버와 클라이언트의 오차 확인 (디버그)
	{
		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		//printf("Distance : %f\n", distance);
	}

	// 클라이언트 보정
	{
		const float SNAP_THRESHOLD = 10.0f;		// 강제
		const float SMOOTH_THRESHOLD = 1.5f;	// 스무스

		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		if (distance > SNAP_THRESHOLD)
		{
			clientEntity->mPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = false;
			// 속도는 동일하니까 패스
		}
		else if (distance > SMOOTH_THRESHOLD && !clientEntity->mIsCorrection)
		{
			clientEntity->mCorrectionStartPosition = clientEntity->mPosition;
			clientEntity->mCorrectionEndPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = true;
			clientEntity->mCorrectionTime = 0.0f;
		}
	}

	std::list<Node*> serverPath;
	for (int index = 0; index < Protocol->PathCount; ++index)
	{
		Waypoint waypoint = Protocol->Path[index];
		serverPath.emplace_back(World->mMap->mGrid[waypoint.GridY][waypoint.GridX].get());
	}

	if (!CompareNodePositions(clientEntity->mPath, serverPath))
	{
		clientEntity->mPath.clear();
		clientEntity->mPath.swap(serverPath);
	}
}

void Handle_PKT_S2C_POSITION_SYNC(const std::unique_ptr<World>& World, const S2C_POSITION* Protocol)
{
	auto clientEntityIter = World->mEntitys.find(Protocol->ObjectId);
	if (clientEntityIter == World->mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& clientEntity = clientEntityIter->second;

	auto serverEntityIter = World->mServerEntitys.find(Protocol->ObjectId);
	if (serverEntityIter == World->mServerEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& serverEntity = serverEntityIter->second;
	serverEntity->mPosition = { Protocol->EntityPositionX, Protocol->EntityPositionY };

	// 지연 시간만큼 예측 적용
	{
		long long serverTimestamp = Protocol->TimeStamp;
		long long clientTimestamp = Time::GetCurrentTimeMs();

		float delayTimestamp = static_cast<float>(clientTimestamp - serverTimestamp) / 1000.0f;
		serverEntity->MoveTowardsNextPath(delayTimestamp);

		printf("[PKT_S2C_POSITION_SYNC]");
		printf("Delay : %f\t", delayTimestamp);
	}

	// 서버와 클라이언트의 오차 확인 (디버그)
	{
		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		printf("Distance : %f\n", distance);
	}

	// 클라이언트 보정
	{
		const float SNAP_THRESHOLD = 10.0f;		// 강제
		const float SMOOTH_THRESHOLD = 1.0f;	// 스무스

		Vector2f clientPosition = clientEntity->mPosition;
		Vector2f serverPosition = serverEntity->mPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		if (distance > SNAP_THRESHOLD)
		{
			clientEntity->mPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = false;
			// 속도는 동일하니까 패스
		}
		else if (distance > SMOOTH_THRESHOLD && !clientEntity->mIsCorrection)
		{
			clientEntity->mCorrectionStartPosition = clientEntity->mPosition;
			clientEntity->mCorrectionEndPosition = serverEntity->mPosition;
			clientEntity->mIsCorrection = true;
			clientEntity->mCorrectionTime = 0.0f;
		}
	}
}

void Handle_S2C_TOGGLE_WALKABLE(const std::unique_ptr<World>& World, const S2C_TOGGLE_WALKABLE* Protocol)
{
	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& node = map->mGrid[Protocol->GridY][Protocol->GridX];

	node->mIsWalkable = Protocol->IsWalkable;

	printf("TOGGLE_WALKABLE Grid[%d][%d] = (%d)\n", Protocol->GridY, Protocol->GridX, node->mIsWalkable);
}
