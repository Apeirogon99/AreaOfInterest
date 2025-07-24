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
	Vector2f gridSize = Vector2f({ 600.0f, 600.0f });
	float nodeSize = 20.0f;
	mMap = std::make_unique<Grid>(gridSize, nodeSize);

	mPathFinder = std::make_unique<PathFinding>();

	mAOI = std::make_unique<AOI>();

	// 더미 봇 생성
	{
		std::random_device rd;
		std::uniform_int_distribution<int> gridX(0, mMap->mGridSizeX - 1);
		std::uniform_int_distribution<int> gridY(0, mMap->mGridSizeY - 1);

		for (int count = 0; count < 1; ++count)
		{
			int entityId = GetNextEntityId();
			const std::unique_ptr<Node>& node = mMap->mGrid[gridX(rd)][gridY(rd)];

			std::unique_ptr<Entity> entity = std::make_unique<Entity>(entityId, node->mPosition);
			entity->mIsAI = true;
			entity->mSessionId = count;

			mEntitys.insert({ entityId, std::move(entity) });
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
	mDirectMessageHandler(SessionId, std::move(message));
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
			std::uniform_real_distribution<float> positionX(-100.0f, 100.0f);
			std::uniform_real_distribution<float> positionY(-100.0f, 100.0f);

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

				auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_PATH_FINDING, protocol);
				mBoradcastMessageHandler(entity->mViewer, std::move(message));
			}
		}
	}

	//////////////////////////////////////////////
	//											//
	//				 가시거리 처리				//
	//											//
	//////////////////////////////////////////////
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

	//////////////////////////////////////////////
	//											//
	//				 동기화 처리					//
	//											//
	//////////////////////////////////////////////
	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		entity->mLastMoveSync += DeltaTime;
		if (entity->mLastMoveSync > entity->mIntervalMoveSync)
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

			entity->mLastMoveSync = 0.0f;
		}
	}

}

int World::GetNextEntityId()
{
	static int ENTITY_ID = 1;
	return ENTITY_ID++;
}
