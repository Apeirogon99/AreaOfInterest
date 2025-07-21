#pragma once
#include "World.h"
#include "Network.h"
#include "memory"

class Game
{
public:
	Game();
	~Game();

public:
	bool Initialize();
	void Destroy();

public:
	void Run();
	void Stop();

private:
	std::unique_ptr<World> mWorld;
	std::unique_ptr<Network> mNetwork;
};
