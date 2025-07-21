#pragma once
#include <functional>
#include <map>

#include "Task.h"
#include "Session.h"

#include "Common/Utils/Time.h"
#include "Common/Game/Entity.h"
#include "Common/AStar/PathFinding.h"
#include "Common/Protocol/Message.h"

class World : public TaskQueue
{
public:
	World();
	~World();

public:
	bool Initialize();
	void Destroy();

public:
	inline bool IsRunning() const { return mIsRunning; }

public:
	void CreateEntity(const std::shared_ptr<Session>& Session);
	void ToggleWalkeable(int GridX, int GridY);

	void PathFind(Entity* Entity, Node* DestNode);
	void PathRefind();

public:
	void SetMessageHandler(std::function<void(std::unique_ptr<Message>)> Handler)
	{
		mMessageHandler = Handler;
	}

public:
	void Update(float DeltaTime);

public:
	bool mIsRunning;

	std::map<uint32_t, std::unique_ptr<Entity>> mEntitys;
	std::unordered_map<uint32_t, uint32_t> mSessionToEntity;

	std::unique_ptr<PathFinding> mPathFinder;
	std::unique_ptr<Grid> mMap;

	// Handler
	std::function<void(std::unique_ptr<Message>)> mMessageHandler;
};