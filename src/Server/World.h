#pragma once
#include <functional>
#include <map>

#include "AOI.h"
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
	void EnterWorld(uint32_t SessionId);
	void LeaveWorld(uint32_t SessionId);
	void PathFind(const uint32_t SessionId, const int DestGridX, const int DestGridY);

public:
	void SetDirectMessageHandler(std::function<void(const uint32_t SessionId, std::unique_ptr<Message>)> Handler)
	{
		mDirectMessageHandler = Handler;
	}

	void SetBoradcastMessageHandler(std::function<void(const std::unordered_set<uint32_t>&, std::unique_ptr<Message>)> Handler)
	{
		mBoradcastMessageHandler = Handler;
	}

public:
	void Update(float DeltaTime);

private:
	int GetNextEntityId();

public:
	bool mIsRunning;

	std::map<uint32_t, std::unique_ptr<Entity>> mEntitys;
	std::unordered_map<uint32_t, uint32_t> mSessionToEntity;

	std::unique_ptr<PathFinding> mPathFinder;
	std::unique_ptr<Grid> mMap;
	std::unique_ptr<AOI> mAOI;

	// Handler
	std::function<void(const uint32_t, std::unique_ptr<Message>)> mDirectMessageHandler;
	std::function<void(const std::unordered_set<uint32_t>&, std::unique_ptr<Message>)> mBoradcastMessageHandler;
};