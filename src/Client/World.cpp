#include "World.h"
#include "ClientMessageHandler.h"

#include <iostream>

#include "SDL2Utils.h"
#include "Common/Utils/Time.h"

World::World() : 
	mIsRunning(false), mWindow(nullptr), mRenderer(nullptr),
	mEntitys(), mLocalEntityId(0),
	mFrequency(0), mLastTime(0)
{
}

World::~World()
{
}

bool World::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return false;
	}

	mWindow = SDL_CreateWindow(
		"Movement Sync",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		600, 600,
		SDL_WINDOW_SHOWN
	);

	if (!mWindow)
	{
		return false;
	}

	mRenderer = SDL_CreateRenderer(
		mWindow,
		-1,
		SDL_RENDERER_ACCELERATED
	);

	if (!mRenderer)
	{
		return false;
	}

	mFrequency = SDL_GetPerformanceFrequency();
	mLastTime = SDL_GetPerformanceCounter();

	// 단순하게 서버와 같다고 생각
	Vector2f gridSize = Vector2f({ 600.0f, 600.0f });
	float nodeSize = 20.0f;
	mMap = std::make_unique<Grid>(gridSize, nodeSize);
	mPathFinder = std::make_unique<PathFinding>();

	mIsRunning = true;
	return true;
}

void World::Destroy()
{
	if (mRenderer)
	{
		SDL_DestroyRenderer(mRenderer);
		mRenderer = nullptr;
	}

	if (mWindow)
	{
		SDL_DestroyWindow(mWindow);
		mWindow = nullptr;
	}

	SDL_Quit();
}

void World::UpdateTime()
{

}

void World::HandleEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;

		case SDL_KEYDOWN:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mIsRunning = false;
				break;
			}
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			Vector2f position = { static_cast<float>(event.button.x), static_cast<float>(event.button.y) };
			Node* node = mMap->GetNodeFromPosition(position);

			auto local = mEntitys.find(mLocalEntityId);
			if (local == mEntitys.end())
			{
				return;
			}
			const std::unique_ptr<Entity>& localEntity = local->second;

			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				{
					localEntity->mPath = mPathFinder->FindPath(mMap, localEntity->mPosition, position);

					C2S_PATH_FINDING protocol;
					protocol.TimeStamp = Time::GetCurrentTimeMs();
					protocol.DestGridPoint = { node->mGridX,  node->mGridY };
	
					std::unique_ptr<Message> message = MessageSerializer::Serialize<C2S_PATH_FINDING>(static_cast<uint16_t>(EMessageId::PKT_C2S_PATH_FINDING), protocol);
					mMessageHandler(std::move(message));
					break;
				}
			}

		break;
		}

		default:
			break;
		}
	}
}

void World::Update()
{
	uint64_t currentTime = SDL_GetPerformanceCounter();
	float deltaTime = static_cast<float>((currentTime - mLastTime)) / mFrequency;
	mLastTime = currentTime;

	static float LastProcessMessageTime = 0.0f;

	LastProcessMessageTime += deltaTime;
	if (LastProcessMessageTime > 10.0f)
	{
		size_t total = 0;
		std::cout << "PROCESS MESSAGE : " << std::endl;
		for (auto iter : gProcessMessageCount)
		{
			std::cout << "ID : " << MessageIdToString(iter.first) << "\tCOUNT : " << iter.second << std::endl;
			total += iter.second;
		}
		std::cout << "TOTAL COUNT : " << total << std::endl << std::endl;
		gProcessMessageCount.clear();
		LastProcessMessageTime = 0.0f;
	}

	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		entity->MoveTowardsNextPath(deltaTime);

		// 보정 필요
		if (entity->mIsCorrection)
		{
			entity->PositionCorrection(deltaTime);
		}
	}
}

void World::Render()
{
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mRenderer);

	// 그리드 그리기
	for (int row = 0; row < mMap->mGridSizeY; row++)
	{
		for (int col = 0; col < mMap->mGridSizeX; col++)
		{
			Node* node = mMap->mGrid[col][row].get();
			// 현재 셀의 화면 위치 계산
			SDL_FRect cellRect;
			cellRect.x = node->mPosition.x - mMap->mNodeHalfSize;
			cellRect.y = node->mPosition.y - mMap->mNodeHalfSize;
			cellRect.w = mMap->mNodeSize;
			cellRect.h = mMap->mNodeSize;

			// 셀의 타입에 따라 다른 색상으로 렌더링
			if (node->mIsWalkable)
			{
				SDL_SetRenderDrawColor(mRenderer, 0, 150, 0, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(mRenderer, 200, 0, 0, 255);
			}

			// 셀 채우기 - 사각형으로 그리기
			SDL_RenderFillRectF(mRenderer, &cellRect);
		}
	}

	// 작은 격자 그리기
	for (int count = 0; count < mMap->mGridSizeX; count++)
	{
		int pos = count * mMap->mNodeSize;
		SDL_SetRenderDrawColor(mRenderer, 100, 100, 100, 255);
		SDL_RenderDrawLine(mRenderer, pos, 0, pos, 600); // 가로
		SDL_RenderDrawLine(mRenderer, 0, pos, 600, pos); // 세로
	}

	// 큰 격자 그리기
	for (int count = 0; count < mMap->mGridSizeX; count++)
	{
		int pos = count * mMap->mNodeSize * 5;
		SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(mRenderer, pos, 0, pos, 600); // 가로
		SDL_RenderDrawLine(mRenderer, 0, pos, 600, pos); // 세로
	}

	// Entity 그리기
	for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
	{
		const uint32_t entityId = iter->first;
		const std::unique_ptr<Entity>& entity = iter->second;

		if (entityId == mLocalEntityId)
		{
			SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
		}

		float entitySize = 10.0f;

		SDL_FRect entityRect;
		entityRect.x = entity->mPosition.x - (entitySize * 0.5f);
		entityRect.y = entity->mPosition.y - (entitySize * 0.5f);
		entityRect.w = entitySize;
		entityRect.h = entitySize;

		SDL_RenderDrawRectF(mRenderer, &entityRect);
	}
	
	auto local = mEntitys.find(mLocalEntityId);
	if (local != mEntitys.end())
	{
		const std::unique_ptr<Entity>& localEntity = local->second;
		const Vector2f& localPosition = localEntity->mPosition;

		// 가시거리 그리기
		SDL_SetRenderDrawColor(mRenderer, 200, 0, 0, 255);
		SDL2Utils::SDL_SetRenderDrawCircleF(mRenderer, localPosition, 75.0f);
		
		SDL_SetRenderDrawColor(mRenderer, 0, 0, 200, 255);
		SDL2Utils::SDL_SetRenderDrawCircleF(mRenderer, localPosition, 150.0f);

		// 경로가 있다면 그리기
		{
			std::list<Node*> path = localEntity->mPath;
			if (path.size() > 0)
			{
				Node* start = *path.begin();

				SDL_SetRenderDrawColor(mRenderer, 255, 51, 153, 255);

				SDL_RenderDrawLine(
					mRenderer,
					localPosition.x, localPosition.y,
					start->mPosition.x, start->mPosition.y);

				for (auto iter = ++path.begin(); iter != path.end(); ++iter)
				{
					Node* end = *iter;

					SDL_RenderDrawLine(
						mRenderer,
						start->mPosition.x, start->mPosition.y,
						end->mPosition.x, end->mPosition.y);

					start = end;
				}
			}
		}
	}

	SDL_RenderPresent(mRenderer);
}

void World::LimitFrameRate()
{
	SDL_Delay(16);
}
