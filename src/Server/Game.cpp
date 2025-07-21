#include "Game.h"
#include "ServerMessageHandler.h"
#include <iostream>

#include "Common/Utils/Time.h"

Game::Game()
{
}

Game::~Game()
{
}

bool Game::Initialize()
{
	mWorld = std::make_unique<World>();
	if (!mWorld->Initialize())
	{
		return false;
	}

	mNetwork = std::make_unique<Network>(9000);
	if (!mNetwork->Initialize())
	{
		return false;
	}

	mWorld->SetMessageHandler([&](std::unique_ptr<Message> Message)
		{
			//std::cout << "Writed" << std::endl;
			mNetwork->Broadcast(std::move(Message));
		});

	mNetwork->SetMessageHandler([&](const std::shared_ptr<Session>& Session, std::unique_ptr<Message> Message)
		{
			//std::cout << "Received" << std::endl;
			ServerMessageHandler::ProcessMessage(mWorld, Session, std::move(Message));
		});

	mNetwork->SetConnectHandler([&](const std::shared_ptr<Session>& Session)
		{
			std::cout << "Connected to client" << std::endl;
			mWorld->CreateEntity(Session);
		});

	mNetwork->SetDisconnectHandler([&](const std::shared_ptr<Session>& Session, boost::system::error_code ec)
		{
			std::cout << "Disconnected from server : " << ec.message() << std::endl;
			
		});

	mNetwork->AcceptAsync();

	return true;
}

void Game::Destroy()
{
	if (mWorld)
	{
		mWorld->Destroy();
	}

	if (mNetwork)
	{
		mNetwork->Destroy();
	}
}

void Game::Run()
{
	long long lastTickTime = Time::GetCurrentTimeMs();
	const long long tickIntervalMs = 33; // 20fps

	while (mNetwork->IsRunning() && mWorld->IsRunning())
	{
		long long currentTime = Time::GetCurrentTimeMs();

		mWorld->ExecuteTasks(currentTime);

		if (currentTime - lastTickTime >= tickIntervalMs)
		{
			float deltaTime = (currentTime - lastTickTime) / 1000.0f;

			mWorld->Update(deltaTime);
			lastTickTime = currentTime;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	Stop();
}

void Game::Stop()
{
	Destroy();
}