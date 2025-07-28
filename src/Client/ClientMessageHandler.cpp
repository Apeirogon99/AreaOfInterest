#include "ClientMessageHandler.h"
#include "Common/Utils/Time.h"

void Handle_INVALID(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data)
{
}

void Handle_PKT_S2C_SPAWN_ENTITY(const std::unique_ptr<World>& World, const S2C_SPAWN_ENTITY* Protocol)
{
	const int objectId = Protocol->ObjectId;
	const GridPoint& spawnGridPoint = Protocol->SpawnGridPoint;

	const std::unique_ptr<Grid>& map = World->mMap;
	const std::unique_ptr<Node>& node = map->mGrid[spawnGridPoint.Y][spawnGridPoint.X];

	World->mLocalEntityId = objectId;
	World->mEntitys.insert({ objectId, std::make_unique<Entity>(objectId, node->mPosition) });

	printf("CreateEntity(%d) [%d][%d]\n", objectId, node->mGridY, node->mGridX);
}

void Handle_PKT_S2C_APPEAR_ENTITY(const std::unique_ptr<World>& World, const S2C_APPEAR_ENTITY* Protocol)
{
	const uint32_t timestamp = Protocol->TimeStamp;
	const int objectId = Protocol->ObjectId;
	const Vector2f& serverEntityPosition = Protocol->EntityPosition;

	auto otherEntityIter = World->mEntitys.find(objectId);
	if (otherEntityIter == World->mEntitys.end())
	{
		World->mEntitys.insert({ objectId, std::make_unique<Entity>(objectId, serverEntityPosition) });
	}


	C2S_ENTITY_INFO protocol;
	protocol.InfoNumber = EEntityInfoPriority::ENTITY_INFO_APPEAR;

	std::unique_ptr<Message> message = MessageSerializer::Serialize(EMessageId::PKT_C2S_ENTITY_INFO, protocol);
	World->mMessageHandler(std::move(message));
}

void Handle_PKT_S2C_DISAPPEAR_ENTITY(const std::unique_ptr<World>& World, const S2C_DISAPPEAR_ENTITY* Protocol)
{
	const int objectId = Protocol->ObjectId;

	if (objectId == 101)
	{
		printf("WHY?\n");
	}

	auto otherEntityIter = World->mEntitys.find(objectId);
	if (otherEntityIter != World->mEntitys.end())
	{
		World->mEntitys.erase(objectId);
	}
}

void Handle_PKT_S2C_ENTITY_INFO(const std::unique_ptr<World>& World, const S2C_ENTITY_INFO* Protocol)
{
	// 인포에 대한 처리
	//Protocol->Infos;

	if (Protocol->InfoNumber >= EEntityInfoPriority::ENTITY_INFO_MAX)
	{
		return;
	}

	C2S_ENTITY_INFO protocol;
	protocol.InfoNumber = Protocol->InfoNumber;

	std::unique_ptr<Message> message = MessageSerializer::Serialize(EMessageId::PKT_C2S_ENTITY_INFO, protocol);
	World->mMessageHandler(std::move(message));
}

void Handle_PKT_S2C_PATH_FINDING(const std::unique_ptr<World>& World, const S2C_PATH_FINDING* Protocol)
{
	const uint32_t timestamp = Protocol->TimeStamp;
	const int32_t objectId = Protocol->ObjectId;

	auto clientEntityIter = World->mEntitys.find(objectId);
	if (clientEntityIter == World->mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& clientEntity = clientEntityIter->second;

	// 지연 시간만큼 예측 적용
	{
		long long serverTimestamp = Protocol->TimeStamp;
		long long clientTimestamp = Time::GetCurrentTimeMs();

		float delayTimestamp = static_cast<float>(clientTimestamp - serverTimestamp) / 1000.0f;
		clientEntity->MoveTowardsNextPath(delayTimestamp);
	}

	// 클라이언트 보정
	{
		const float SNAP_THRESHOLD = 10.0f;		// 강제
		const float SMOOTH_THRESHOLD = 1.5f;	// 스무스

		const Vector2f& clientPosition = clientEntity->mPosition;
		const Vector2f& serverPosition = Protocol->EntityPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		if (distance > SNAP_THRESHOLD)
		{
			clientEntity->mPosition = serverPosition;
			clientEntity->mIsCorrection = false;
			// 속도는 동일하니까 패스
		}
		else if (distance > SMOOTH_THRESHOLD && !clientEntity->mIsCorrection)
		{
			clientEntity->mCorrectionStartPosition = clientEntity->mPosition;
			clientEntity->mCorrectionEndPosition = serverPosition;
			clientEntity->mIsCorrection = true;
			clientEntity->mCorrectionTime = 0.0f;
		}
	}

	std::list<Node*> serverPath;
	for (int index = 0; index < Protocol->PathCount; ++index)
	{
		const GridPoint& waypoint = Protocol->Path[index];
		serverPath.emplace_back(World->mMap->mGrid[waypoint.Y][waypoint.X].get());
	}

	if (!CompareNodePositions(clientEntity->mPath, serverPath))
	{
		clientEntity->mPath.clear();
		clientEntity->mPath.swap(serverPath);
	}
}

void Handle_PKT_S2C_POSITION_SYNC(const std::unique_ptr<World>& World, const S2C_POSITION* Protocol)
{
	const uint32_t timestamp = Protocol->TimeStamp;
	const int32_t objectId = Protocol->ObjectId;

	auto clientEntityIter = World->mEntitys.find(objectId);
	if (clientEntityIter == World->mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& clientEntity = clientEntityIter->second;

	// 지연 시간만큼 예측 적용
	{
		long long serverTimestamp = Protocol->TimeStamp;
		long long clientTimestamp = Time::GetCurrentTimeMs();

		float delayTimestamp = static_cast<float>(clientTimestamp - serverTimestamp) / 1000.0f;
		clientEntity->MoveTowardsNextPath(delayTimestamp);
	}

	// 클라이언트 보정
	{
		const float SNAP_THRESHOLD = 10.0f;		// 강제
		const float SMOOTH_THRESHOLD = 1.0f;	// 스무스

		const Vector2f& clientPosition = clientEntity->mPosition;
		const Vector2f& serverPosition = Protocol->EntityPosition;
		Vector2f direction = serverPosition - clientPosition;
		float distance = direction.magnitude();

		if (distance > SNAP_THRESHOLD)
		{
			clientEntity->mPosition = serverPosition;
			clientEntity->mIsCorrection = false;
		}
		else if (distance > SMOOTH_THRESHOLD && !clientEntity->mIsCorrection)
		{
			clientEntity->mCorrectionStartPosition = clientEntity->mPosition;
			clientEntity->mCorrectionEndPosition = serverPosition;
			clientEntity->mIsCorrection = true;
			clientEntity->mCorrectionTime = 0.0f;
		}
	}
}