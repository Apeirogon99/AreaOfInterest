#include "World.h"

#include <iostream>
#include <random>

#include "ServerMessageHandler.h"

World::World() : 
	mIsRunning(false),
	mEntitys()
{
}

World::~World()
{
}

bool World::Initialize()
{
	// 단순하게 클라와 같다고 생각
	Vector2f worldSize = Vector2f({ 600.0f, 600.0f });
	float nodeSize = 20.0f;
	mMap = std::make_unique<Grid>(worldSize, nodeSize);

	mPathFinder = std::make_unique<PathFinding>();

	mAOI = std::make_unique<AOI>(worldSize, nodeSize, 5);

	// 더미 봇 생성
	{
		std::random_device rd;
		std::uniform_int_distribution<int> gridX(0, mMap->mGridSizeX - 1);
		std::uniform_int_distribution<int> gridY(0, mMap->mGridSizeY - 1);

		for (int count = 1000; count < 1100; ++count)
		{
			int entityId = GetNextEntityId();
			const std::unique_ptr<Node>& node = mMap->mGrid[gridX(rd)][gridY(rd)];

			std::unique_ptr<Entity> entity = std::make_unique<Entity>(entityId, node->mPosition);
			entity->mIsAI = true;
			entity->mSessionId = count;

			mEntitys.insert({ entityId, std::move(entity) });
			mSessionToEntity.insert({ count, entityId });
		}
	}

	mIsRunning = true;
	return true;
}

void World::Destroy()
{

}

void World::EnterWorld(uint32_t SessionId)
{
	int entityId = GetNextEntityId();

	Node* node = mMap->GetNodeFromPosition({ 10.0f, 10.0f });
	Vector2f spawnPosition = node->mPosition;

	std::unique_ptr<Entity> newEntity = std::make_unique<Entity>(entityId, spawnPosition);

	{
		newEntity->mSessionId = SessionId;

		newEntity->mViewer.insert(SessionId);
		newEntity->mMonitor.insert(entityId);
	}

	{
		for (auto otherEntityIter = mEntitys.begin(); otherEntityIter != mEntitys.end(); ++otherEntityIter)
		{
			const uint32_t otherEntityId = otherEntityIter->first;
			const std::unique_ptr<Entity>& otherEntity = otherEntityIter->second;

			if (entityId == otherEntityId) continue;

			float distance = newEntity->mPosition.distance(otherEntity->mPosition);
			if (distance < 150.0f)
			{
				{
					newEntity->mViewer.insert(otherEntity->mSessionId);
					otherEntity->mMonitor.insert(entityId);

					S2C_APPEAR_ENTITY protocol;
					protocol.TimeStamp = Time::GetCurrentTimeMs();
					protocol.ObjectId = entityId;
					protocol.EntityPosition = newEntity->mPosition;

					auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_APPEAR_ENTITY, protocol);
					mDirectMessageHandler(otherEntity->mSessionId, std::move(message));
				}

				{
					otherEntity->mViewer.insert(newEntity->mSessionId);
					newEntity->mMonitor.insert(otherEntityId);

					S2C_APPEAR_ENTITY protocol;
					protocol.TimeStamp = Time::GetCurrentTimeMs();
					protocol.ObjectId = otherEntityId;
					protocol.EntityPosition = otherEntity->mPosition;

					auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_APPEAR_ENTITY, protocol);
					mDirectMessageHandler(newEntity->mSessionId, std::move(message));
				}
			}
		}
	}

	{
		mEntitys.insert({ entityId, std::move(newEntity) });
		mSessionToEntity.insert({ SessionId, entityId });
	}

	{
		S2C_SPAWN_ENTITY protocol;
		protocol.ObjectId = entityId;
		protocol.SpawnGridPoint = { node->mGridX, node->mGridY };

		auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_SPAWN_ENTITY, protocol);
		mDirectMessageHandler(SessionId, std::move(message));
	}

	printf("CreateEntity(%d) [%d][%d]\n", entityId, node->mGridY, node->mGridX);

}

void World::LeaveWorld(uint32_t SessionId)
{
}

void World::PathFind(const uint32_t SessionId, const int DestGridX, const int DestGridY)
{
	auto sessionIter = mSessionToEntity.find(SessionId);
	if (sessionIter == mSessionToEntity.end())
	{
		return;
	}

	auto entityIter = mEntitys.find(sessionIter->second);
	if (entityIter == mEntitys.end())
	{
		return;
	}
	const std::unique_ptr<Entity>& entity = entityIter->second;
	const std::unique_ptr<Node>& node = mMap->mGrid[DestGridY][DestGridX];

	std::list<Node*> paths = mPathFinder->FindPath(mMap, entity->mPosition, node->mPosition);

	S2C_PATH_FINDING protocol;
	protocol.TimeStamp = Time::GetCurrentTimeMs();
	protocol.ObjectId = entity->mObjectId;
	protocol.EntityPosition = entity->mPosition;
	protocol.PathCount = 0;

	entity->mPath.clear();
	for (Node* path : paths)
	{
		protocol.Path[protocol.PathCount++] = GridPoint({ path->mGridX, path->mGridY });
		entity->mPath.emplace_back(path);
	}

	auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_PATH_FINDING, protocol);
	mBoradcastMessageHandler(entity->mViewer, std::move(message));
}

void World::Update(float DeltaTime)
{
	//////////////////////////////////////////////
	//											//
	//				   이동 처리					//
	//											//
	//////////////////////////////////////////////
	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		// DeltaTime 만큼 이동
		entity->MoveTowardsNextPath(DeltaTime);

		// AI이고 경로 없으면 새로 지정해주기
		if (entity->mIsAI && entity->mPath.empty())
		{
			std::random_device rd;
			std::uniform_real_distribution<float> positionX(-50.0f, 50.0f);
			std::uniform_real_distribution<float> positionY(-50.0f, 50.0f);

			const Vector2f& entityPosition = entity->mPosition;
			const Vector2f destPosition = entityPosition - Vector2f({ positionX(rd), positionY(rd) });

			entity->mPath = mPathFinder->FindPath(mMap, entityPosition, destPosition);

			{
				S2C_PATH_FINDING protocol;
				protocol.TimeStamp = Time::GetCurrentTimeMs();
				protocol.ObjectId = entityId;
				protocol.EntityPosition = entityPosition;
				protocol.PathCount = 0;

				for (Node* node : entity->mPath)
				{
					protocol.Path[protocol.PathCount++] = GridPoint({ node->mGridX, node->mGridY });
				}

#if USE_AOI
				std::unordered_set<uint32_t> nearViewers;
				for (uint32_t monitorEntityId : entity->mMonitor)
				{
					const std::unique_ptr<Entity>& viewerEntity = mEntitys[monitorEntityId];
					float distance = entity->mPosition.distance(viewerEntity->mPosition);

					if (distance <= 50.0f)
					{
						nearViewers.insert(viewerEntity->mSessionId);
					}
				}

				auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_PATH_FINDING, protocol);
				mBoradcastMessageHandler(nearViewers, std::move(message));
#else
				auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_PATH_FINDING, protocol);
				mBoradcastMessageHandler(entity->mViewer, std::move(message));
#endif // USE_AOI

				
			}
		}
	}

	//////////////////////////////////////////////
	//											//
	//				 가시거리 처리				//
	//											//
	//////////////////////////////////////////////

#if USE_AOI
	/// <summary>
	/// 그리드 형식으로 검사
	/// </summary>
	mAOI->PlaceEntityInGroup(mEntitys);

	for (auto entityIter = mEntitys.begin(); entityIter != mEntitys.end(); ++entityIter)
	{
		const uint32_t entityId = entityIter->first;
		const std::unique_ptr<Entity>& entity = entityIter->second;

		std::unordered_set<uint32_t> visibleEntities;
		std::list<NodeGroup*> groups = mAOI->GetNodeGroupInRange(entity->mPosition, 150.0f);
		for (NodeGroup* group : groups)
		{
			for (const uint32_t otherEntityId : group->mEntityIds)
			{
				if (entityId == otherEntityId) continue;

				const std::unique_ptr<Entity>& otherEntity = mEntitys[otherEntityId];
				float distance = entity->mPosition.distance(otherEntity->mPosition);
				if (distance < 150.0f)
				{
					visibleEntities.insert(otherEntity->mSessionId);

					if (entity->mViewer.find(otherEntity->mSessionId) == entity->mViewer.end())
					{
						entity->mViewer.insert(otherEntity->mSessionId);
						otherEntity->mMonitor.insert(entityId);

						S2C_APPEAR_ENTITY protocol;
						protocol.TimeStamp = Time::GetCurrentTimeMs();
						protocol.ObjectId = entityId;
						protocol.EntityPosition = entity->mPosition;

						auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_APPEAR_ENTITY, protocol);
						mDirectMessageHandler(otherEntity->mSessionId, std::move(message));
					}
				}
			}
		}

		auto otherSessionIdIter = entity->mViewer.begin();
		while (otherSessionIdIter != entity->mViewer.end())
		{
			const uint32_t otherSessionId = *otherSessionIdIter;
			
			if (otherSessionId != entity->mSessionId && visibleEntities.find(otherSessionId) == visibleEntities.end())
			{
				const uint32_t otherEntityId = mSessionToEntity[otherSessionId];
				const std::unique_ptr<Entity>& otherEntity = mEntitys[otherEntityId];

				S2C_DISAPPEAR_ENTITY protocol;
				protocol.ObjectId = entityId;

				auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_DISAPPEAR_ENTITY, protocol);
				mDirectMessageHandler(otherEntity->mSessionId, std::move(message));

				otherEntity->mMonitor.erase(entityId);
				otherSessionIdIter = entity->mViewer.erase(otherSessionIdIter);
			}
			else
			{
				++otherSessionIdIter;
			}
		}
	}
#else
	//<summary>
	//2중 for문으로 검사
	//</summary>
	for (auto entityIter = mEntitys.begin(); entityIter != mEntitys.end(); ++entityIter)
	{
		const uint32_t entityId = entityIter->first;
		const std::unique_ptr<Entity>& entity = entityIter->second;

		for (auto otherEntityIter = mEntitys.begin(); otherEntityIter != mEntitys.end(); ++otherEntityIter)
		{
			const uint32_t otherEntityId = otherEntityIter->first;
			const std::unique_ptr<Entity>& otherEntity = otherEntityIter->second;

			if (entityId == otherEntityId) continue;

			float distance = entity->mPosition.distance(otherEntity->mPosition);
			if (distance < 150.0f)
			{
				if (entity->mViewer.find(otherEntity->mSessionId) == entity->mViewer.end())
				{
					entity->mViewer.insert(otherEntity->mSessionId);
					otherEntity->mMonitor.insert(entityId);

					S2C_APPEAR_ENTITY protocol;
					protocol.TimeStamp = Time::GetCurrentTimeMs();
					protocol.ObjectId = entityId;
					protocol.EntityPosition = entity->mPosition;

					auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_APPEAR_ENTITY, protocol);
					mDirectMessageHandler(otherEntity->mSessionId, std::move(message));
				}
			}
			else
			{
				if (entity->mViewer.find(otherEntity->mSessionId) != entity->mViewer.end())
				{
					entity->mViewer.erase(otherEntity->mSessionId);
					otherEntity->mMonitor.erase(entityId);

					S2C_DISAPPEAR_ENTITY protocol;
					protocol.ObjectId = entityId;

					auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_DISAPPEAR_ENTITY, protocol);
					mDirectMessageHandler(otherEntity->mSessionId, std::move(message));
				}
			}
		}
	}
#endif // USE_AOI

	//////////////////////////////////////////////
	//											//
	//				 동기화 처리					//
	//											//
	//////////////////////////////////////////////

#if USE_AOI
	//<summary>
	// 거리에 따라 정보 및 주기 조절
	//</summary>
	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		entity->mLastNearSync += DeltaTime;
		entity->mLastFarSync += DeltaTime;

		if (entity->mLastNearSync > 0.25f)
		{
			S2C_POSITION protocol;
			protocol.TimeStamp = Time::GetCurrentTimeMs();
			protocol.ObjectId = entityId;
			protocol.EntityPosition = entity->mPosition;
			if (!entity->mPath.empty())
			{
				Node* destNode = entity->mPath.back();
				protocol.DestGridPoint = { destNode->mGridX, destNode->mGridY };
			}

			std::unordered_set<uint32_t> nearViewers;
			for (uint32_t monitorEntityId : entity->mMonitor)
			{
				const std::unique_ptr<Entity>& viewerEntity = mEntitys[monitorEntityId];
				float distance = entity->mPosition.distance(viewerEntity->mPosition);

				if (distance <= 50.0f)
				{
					nearViewers.insert(viewerEntity->mSessionId);
				}
			}

			auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_POSITION_SYNC, protocol);
			mBoradcastMessageHandler(nearViewers, std::move(message));

			entity->mLastNearSync = 0.0f;
		}
		
		if (entity->mLastFarSync > 1.0f)
		{
			S2C_POSITION protocol;
			protocol.TimeStamp = Time::GetCurrentTimeMs();
			protocol.ObjectId = entityId;
			protocol.EntityPosition = entity->mPosition;

			std::unordered_set<uint32_t> farViewers;
			for (uint32_t monitorEntityId : entity->mMonitor)
			{
				const std::unique_ptr<Entity>& viewerEntity = mEntitys[monitorEntityId];
				float distance = entity->mPosition.distance(viewerEntity->mPosition);

				if (50.0f < distance && distance <= 150.0f)
				{
					farViewers.insert(viewerEntity->mSessionId);
				}
			}

			auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_POSITION_SYNC, protocol);
			mBoradcastMessageHandler(farViewers, std::move(message));

			entity->mLastFarSync = 0.0f;
		}
	}

#else
	//<summary>
	// 모든 정보를 0.25초마다 보내기
	//</summary>
	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		entity->mSingleLastSync += DeltaTime;
		if (entity->mSingleLastSync > 0.25f)
		{
			S2C_POSITION protocol;
			protocol.TimeStamp = Time::GetCurrentTimeMs();
			protocol.ObjectId = entityId;
			protocol.EntityPosition = entity->mPosition;
			if (!entity->mPath.empty())
			{
				Node* destNode = entity->mPath.back();
				protocol.DestGridPoint = { destNode->mGridX, destNode->mGridY };
			}

			auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_POSITION_SYNC, protocol);
			mBoradcastMessageHandler(entity->mViewer, std::move(message));

			entity->mSingleLastSync = 0;
		}
	}
#endif // USE_AOI

}

int World::GetNextEntityId()
{
	static int ENTITY_ID = 1;
	return ENTITY_ID++;
}
