#pragma once
#include <SDL.h>
#include <map>

#include "Network.h"

#include "Common/Game/Entity.h"
#include "Common/AStar/PathFinding.h"

class World
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
	void SetMessageHandler(std::function<void(std::unique_ptr<Message>)> Handler)
	{
		mMessageHandler = Handler;
	}

public:
	void UpdateTime();
	void HandleEvents();
	void Update();
	void Render();
	void LimitFrameRate();



private:
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;

public:
	bool mIsRunning;
	uint64_t mFrequency;
	uint64_t mLastTime;

	uint32_t mLocalEntityId;
	std::map<uint32_t, std::unique_ptr<Entity>> mEntitys;

	std::unique_ptr<PathFinding> mPathFinder;
	std::unique_ptr<Grid> mMap;

	// Handler
	std::function<void(std::unique_ptr<Message> Message)> mMessageHandler;
};