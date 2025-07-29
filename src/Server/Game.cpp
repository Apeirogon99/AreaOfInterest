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

	mWorld->SetDirectMessageHandler([&](const uint32_t SessionId, std::unique_ptr<Message> Message)
		{
			mNetwork->Direct(SessionId, std::move(Message));
		});

	mWorld->SetBoradcastMessageHandler([&](const std::unordered_set<uint32_t>& SessionIds, std::unique_ptr<Message> Message)
		{
			mNetwork->Broadcast(SessionIds, std::move(Message));
		});

	mNetwork->SetMessageHandler([&](const std::shared_ptr<Session>& Session, std::unique_ptr<Message> Message)
		{
			ServerMessageHandler::ProcessMessage(mWorld, Session, std::move(Message));
		});

	mNetwork->SetConnectHandler([&](const std::shared_ptr<Session>& Session)
		{
			std::cout << "Connected to client" << std::endl;
			mWorld->PushTask(0, mWorld, &World::EnterWorld, Session->GetSessionId());
		});

	mNetwork->SetDisconnectHandler([&](const std::shared_ptr<Session>& Session, boost::system::error_code ec)
		{
			std::cout << "Disconnected from server : " << ec.message() << std::endl;
			mWorld->PushTask(0, mWorld, &World::LeaveWorld, Session->GetSessionId());
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

	const long long FRAME = 33; // 20fps;
	gTimeManager.Initialize();
	long long lastTickTime = gTimeManager.GetServerTime();

	while (mNetwork->IsRunning() && mWorld->IsRunning())
	{
		long long currentTime = gTimeManager.GetServerTime();

		mWorld->ExecuteTasks(currentTime);

		if (currentTime - lastTickTime >= FRAME)
		{
			float deltaTime = (currentTime - lastTickTime) / 1000.0f;

			mWorld->Update(gTimeManager.GetDeltaTime());
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