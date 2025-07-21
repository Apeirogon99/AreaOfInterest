#include "World.h"
#include "ServerMessageHandler.h"

#include <iostream>

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

	mIsRunning = true;
	return true;
}

void World::Destroy()
{

}

void World::CreateEntity(const std::shared_ptr<Session>& Session)
{
	static int ENTITY_ID = 1;

	Node* node = mMap->GetNodeFromPosition({ 10.0f, 10.0f });
	Vector2f spawnPosition = node->mPosition;

	std::unique_ptr<Entity> newEntity = std::make_unique<Entity>(ENTITY_ID, spawnPosition);
	printf("CreateEntity(%d) [%d][%d]\n", ENTITY_ID, node->mGridY, node->mGridX);

	mEntitys.insert({ ENTITY_ID, std::move(newEntity) });
	mSessionToEntity.insert({Session->GetSessionId(), ENTITY_ID });

	S2C_SPAWN_ENTITY protocol;
	protocol.ObjectId = ENTITY_ID++;
	protocol.GridX = node->mGridX;
	protocol.GridY = node->mGridY;

	std::unique_ptr<Message> message = MessageSerializer::Serialize<S2C_SPAWN_ENTITY>(static_cast<uint16_t>(EMessageId::PKT_S2C_SPAWN_ENTITY), protocol);
	Session->Write(std::move(message));
}

void World::ToggleWalkeable(int GridX, int GridY)
{
	const std::unique_ptr<Node>& node = mMap->mGrid[GridY][GridX];
	node->mIsWalkable = !node->mIsWalkable;

	S2C_TOGGLE_WALKABLE protocol;
	protocol.GridX = node->mGridX;
	protocol.GridY = node->mGridY;
	protocol.IsWalkable = node->mIsWalkable;

	printf("TOGGLE_WALKABLE Grid[%d][%d] (%d)->(%d)\n", GridY, GridX, !node->mIsWalkable, node->mIsWalkable);

	std::unique_ptr<Message> message = MessageSerializer::Serialize<S2C_TOGGLE_WALKABLE>(static_cast<uint16_t>(EMessageId::PKT_S2C_TOGGLE_WALKABLE), protocol);
	mMessageHandler(std::move(message));
}

void World::PathFind(Entity* Entity, Node* DestNode)
{
	if (Entity == nullptr || DestNode == nullptr)
	{
		return;
	}

	std::list<Node*> path = mPathFinder->FindPath(mMap, Entity->mPosition, DestNode->mPosition);

	// 방향이 급격하게 전환된 경우
	if (!Entity->mPath.empty() && !path.empty())
	{
		Node* curNode = mMap->GetNodeFromPosition(Entity->mPosition);
		Node* oldNode = Entity->mPath.front();
		Node* nextNode = path.front();

		GridDirection currentDirection = Grid::GetGridDirection(curNode, oldNode);
		GridDirection nextDirection = Grid::GetGridDirection(curNode, nextNode);

		if (Grid::IsAngleExceeded(currentDirection, nextDirection, 90))
		{
			// 즉시 동기화 주기를 낮춤
			Entity->mIntervalMoveSync = 0.1f;
		}
	}

	S2C_PATH_FINDING protocol;
	protocol.TimeStamp = Time::GetCurrentTimeMs();
	protocol.ObjectId = Entity->mObjectId;
	protocol.EntityPositionX = Entity->mPosition.x;
	protocol.EntityPositionY = Entity->mPosition.y;
	protocol.PathCount = 0;

	Entity->mPath.clear();
	for (Node* node : path)
	{
		protocol.Path[protocol.PathCount++] = Waypoint({ node->mGridX, node->mGridY });
		Entity->mPath.emplace_back(node);
	}

	std::unique_ptr<Message> message = MessageSerializer::Serialize<S2C_PATH_FINDING>(static_cast<uint16_t>(EMessageId::PKT_S2C_PATH_FINDING), protocol);
	mMessageHandler(std::move(message));
}

void World::PathRefind()
{
	for (auto entityIter = mEntitys.begin(); entityIter != mEntitys.end(); ++entityIter)
	{
		const uint32_t entityId = entityIter->first;
		const std::unique_ptr<Entity>& entity = entityIter->second;

		if (entity->mPath.empty()) continue;

		std::list<Node*> oldPath = entity->mPath;
		std::list<Node*> newPath = mPathFinder->FindPath(mMap, oldPath.front()->mPosition, oldPath.back()->mPosition);

		if (false == CompareNodePositions(oldPath, newPath))
		{
			S2C_PATH_REFINDING protocol;
			protocol.TimeStamp = 0;
			protocol.ObjectId = entityId;
			protocol.PathCount = 0;

			entity->mPath.clear();
			for (Node* node : newPath)
			{
				protocol.Path[protocol.PathCount++] = Waypoint({ node->mGridX, node->mGridY });
				entity->mPath.emplace_back(node);
			}

			std::unique_ptr<Message> message = MessageSerializer::Serialize<S2C_PATH_REFINDING>(static_cast<uint16_t>(EMessageId::PKT_S2C_PATH_REFINDING), protocol);
			mMessageHandler(std::move(message));
		}
	}
	
}

void World::Update(float DeltaTime)
{
	static int CheckSendSyncPacket = 0;
	static float LastCheckSendPacket = 0.0f;

	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		entity->MoveTowardsNextPath(DeltaTime);

		entity->mLastMoveSync += DeltaTime;
		if (entity->mLastMoveSync > entity->mIntervalMoveSync)
		{
			CheckSendSyncPacket++;

			// 주기 회복
			//printf("%f\n", entity->mIntervalMoveSync);
			entity->RecoveyIntervalMoveSync();

			S2C_POSITION protocol;
			protocol.TimeStamp = Time::GetCurrentTimeMs();
			protocol.ObjectId = entityId;
			protocol.EntityPositionX = entity->mPosition.x;
			protocol.EntityPositionY = entity->mPosition.y;
			if (entity->GetDestinationNode() != nullptr)
			{
				protocol.NextGridX = entity->GetDestinationNode()->mGridX;
				protocol.NextGridY = entity->GetDestinationNode()->mGridY;
			}

			std::unique_ptr<Message> message = MessageSerializer::Serialize<S2C_POSITION>(static_cast<uint16_t>(EMessageId::PKT_S2C_POSITION_SYNC), protocol);
			mMessageHandler(std::move(message));

			entity->mLastMoveSync = 0.0f;
		}

		LastCheckSendPacket += DeltaTime;
		if (LastCheckSendPacket >= 1.0f)
		{
			printf("CHECK : %d\n", CheckSendSyncPacket);
			LastCheckSendPacket = 0.0f;
		}
	}
}