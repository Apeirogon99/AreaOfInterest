#include "Game.h"
#include "ClientMessageHandler.h"
#include <iostream>

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

	mNetwork = std::make_unique<Network>("localhost", 9000);
	if (!mNetwork->Initialize())
	{
		return false;
	}

	mWorld->SetMessageHandler([&](std::unique_ptr<Message> Message)
		{
			//std::cout << "Writed" << std::endl;
			mNetwork->Write(std::move(Message));
		});

	mNetwork->SetMessageHandler([&](std::unique_ptr<Message> Message)
		{
			//std::cout << "Received" << std::endl;
			ClientMessageHandler::ProcessMessage(mWorld, std::move(Message));
		});

	mNetwork->SetConnectHandler([&]()
		{
			std::cout << "Connected to server" << std::endl;

		});

	mNetwork->SetDisconnectHandler([&](boost::system::error_code ec)
		{
			std::cout << "Disconnected from server : " << ec.message() << std::endl;
			mNetwork->Disconnect();
		});

	mNetwork->Connect();

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
	// 네트워크 포함
	while (mWorld->IsRunning())
	{
		mWorld->HandleEvents();		// 입력 처리

		mNetwork->PollMessage();	// 네트워크 메세지 처리

		mWorld->Update();			// 로직 업데이트

		mWorld->Render();			// 렌더링

		mWorld->LimitFrameRate();	// 60FPS
	}

	Stop();
}

void Game::Stop()
{
	Destroy();
}
