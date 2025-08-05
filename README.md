# AreaOfInterest
AreaOfInterest는 MMORPG 프로젝트 제작 중 가시거리의 일부 문제를 해결하고자 만든 간단한 프로젝트입니다.

## 목적
### 플레이어 시야밖 불필요한 정보와 동기

가시거리의 특성상 플레이어 화면에 없다가 갑작스럽게 나타나게 된다면 <br>
유저는 어색함을 느끼게 되어 화면보다는 좀 더 큰 범위에 영역을 설정합니다. <br>

문제는 자연스러움을 위해 화면에 보이지 않는 플레이어들의 정보를 모두 동기화 <br>
하는 것은 또 다시 불필요한 트래픽이 발생하고 있어 해결하고자 하였습니다. <br>

모든 정보가 플레이어 시야 밖에서도 동일하다는 것이 문제였습니다. <br>
때문에 플레이어 시야를 기준으로 가시영역을 나누었습니다. <br>

영역에 따라 정보와 동기화 주기를 다르게 설정하였습니다. <br>
큰 영역은 플레이어와 존재와 필수 정보(위치)와 느린 동기화 주기를 <br>
작은 영역은 모든 정보와 빠른 동기화로 트래픽을 감소 시켰습니다. <br>

## 테스트 환경
<img width="468" height="464" alt="image" src="https://github.com/user-attachments/assets/6162d536-2b76-41bd-baca-098f6969192e" />

## 결과 요약
가시영역 범위와 정보와 동기화에 따라 더 큰 효과를 볼 수 있을거라고 생각합니다. <br>

현재 상황에서는 트래픽 62% 절약하면서도 품질을 유지하였습니다. <br>

<img width="791" height="222" alt="image" src="https://github.com/user-attachments/assets/e1d21f3a-fd89-440e-ad69-4ee9647c77d5" />

## 결과 GIF
### 이중 가시영역
![이중 가시영역](https://github.com/user-attachments/assets/a4d91460-59ad-4ad0-85b6-0ac1c43cabf9)

### 단일 가시영역
![단일 가시영역](https://github.com/user-attachments/assets/30a8bb9e-cb66-4c0c-8961-55561a047736)

## 주요 코드

### 거리에 따른 동기화 및 정보

고정된 숫자로 75(작은 범위) 150(큰 범위)로 나누어져 있습니다. </br>
거리에 따라 75이면 near viewer, 150이면 far viewer로하여 각각 정보를 동기화 주기에 맞게 전달합니다. </br>
동기화 주기는 near는 0.25초, far는 1초마다 보내집니다. </br>
동기화 정보는 near는 위치 + 경로, far는 위치를 동기화 합니다. </br>

이렇게 한다면 트래픽을 불필요한 정보를 주는 것을 방지할 수 있다고 생각하였습니다. </br>

```
//<summary>
// 거리에 따라 정보 및 주기 조절
//</summary>
for (auto iter = mEntitys.begin(); iter != mEntitys.end(); ++iter)
{
	const uint32_t entityId = iter->first;
	const std::unique_ptr<Entity>& entity = iter->second;

	entity->mLastNearSync += DeltaTime;
	entity->mLastFarSync += DeltaTime;

	if (entity->mLastNearSync > 0.25f)
	{
		S2C_POSITION protocol;
		protocol.TimeStamp = gTimeManager.GetServerTime();
		protocol.ObjectId = entityId;
		protocol.EntityPosition = entity->mPosition;
		if (!entity->mPath.empty())
		{
			Node* destNode = entity->mPath.back();
			protocol.DestGridPoint = { destNode->mGridX, destNode->mGridY };
		}

		std::unordered_set<uint32_t> nearViewers;
		for (uint32_t monitorEntityId : entity->mMonitor)
		{
			const std::unique_ptr<Entity>& viewerEntity = mEntitys[monitorEntityId];
			float distance = entity->mPosition.distance(viewerEntity->mPosition);

			if (distance <= 75.0f)
			{
				nearViewers.insert(viewerEntity->mSessionId);
			}
		}

		auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_POSITION_SYNC, protocol);
		mBoradcastMessageHandler(nearViewers, std::move(message));

		entity->mLastNearSync = 0.0f;
	}
	
	if (entity->mLastFarSync > 1.0f)
	{
		S2C_POSITION protocol;
		protocol.TimeStamp = gTimeManager.GetServerTime();
		protocol.ObjectId = entityId;
		protocol.EntityPosition = entity->mPosition;

		std::unordered_set<uint32_t> farViewers;
		for (uint32_t monitorEntityId : entity->mMonitor)
		{
			const std::unique_ptr<Entity>& viewerEntity = mEntitys[monitorEntityId];
			float distance = entity->mPosition.distance(viewerEntity->mPosition);

			if (75.0f < distance && distance <= 150.0f)
			{
				farViewers.insert(viewerEntity->mSessionId);
			}
		}

		auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_POSITION_SYNC, protocol);
		mBoradcastMessageHandler(farViewers, std::move(message));

		entity->mLastFarSync = 0.0f;
	}
}
```

### 그리드 영역

기존 가시영역은 모든 객체를 2중 반복문을 통해 영역에 넣었습니다.  </br>
이는 더 많은 인원을 추가할 경우 프레임안에 처리하기 어렵다고 판단하여 그리드를 추가하여 시간복잡도를 줄이고자 노력하였습니다다 </br>
```
// Entity 그리드 영역에 넣기
void AOI::PlaceEntityInGroup(const std::map<uint32_t, std::unique_ptr<Entity>>& Entitys)
{
	for (auto entityIter = Entitys.begin(); entityIter != Entitys.end(); ++entityIter)
	{
		const std::unique_ptr<Entity>& entity = entityIter->second;
		const Vector2f& position = entity->mPosition;

		int groupX = static_cast<int>(floorf(position.x / mGroupSize));
		int groupY = static_cast<int>(floorf(position.y / mGroupSize));

		int groupId = groupY * mGroupCountX + groupX;
		const std::unique_ptr<NodeGroup>& group = mGroups[groupId];

		if (group->mEntityIds.find(entity->mObjectId) == group->mEntityIds.end())
		{
			// 기존 그룹이 있다면 탈퇴
			if (entity->mNodeGroupId != UINT32_MAX)
			{
				const std::unique_ptr<NodeGroup>& oldGroup = mGroups[entity->mNodeGroupId];
				oldGroup->mEntityIds.erase(entity->mObjectId);
			}

			entity->mNodeGroupId = groupId;
			group->mEntityIds.insert(entity->mObjectId);
		}
	}
}
```

```
// 그리드 영역 넣기
mAOI->PlaceEntityInGroup(mEntitys);

// 영역에 보인다면 추가하고 삭제하기
for (auto entityIter = mEntitys.begin(); entityIter != mEntitys.end(); ++entityIter)
{
	const uint32_t entityId = entityIter->first;
	const std::unique_ptr<Entity>& entity = entityIter->second;

	std::unordered_set<uint32_t> visibleEntities;
	std::list<NodeGroup*> groups = mAOI->GetNodeGroupInRange(entity->mPosition, 150.0f);
	for (NodeGroup* group : groups)
	{
		for (const uint32_t otherEntityId : group->mEntityIds)
		{
			if (entityId == otherEntityId) continue;

			const std::unique_ptr<Entity>& otherEntity = mEntitys[otherEntityId];
			float distance = entity->mPosition.distance(otherEntity->mPosition);
			if (distance < 150.0f)
			{
				visibleEntities.insert(otherEntity->mSessionId);

				if (entity->mViewer.find(otherEntity->mSessionId) == entity->mViewer.end())
				{
					entity->mViewer.insert(otherEntity->mSessionId);
					otherEntity->mMonitor.insert(entityId);

					S2C_APPEAR_ENTITY protocol;
					protocol.TimeStamp = gTimeManager.GetServerTime();
					protocol.ObjectId = entityId;
					protocol.EntityPosition = entity->mPosition;

					auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_APPEAR_ENTITY, protocol);
					mDirectMessageHandler(otherEntity->mSessionId, std::move(message));
				}
			}
		}
	}

	auto otherSessionIdIter = entity->mViewer.begin();
	while (otherSessionIdIter != entity->mViewer.end())
	{
		const uint32_t otherSessionId = *otherSessionIdIter;
		
		if (otherSessionId != entity->mSessionId && visibleEntities.find(otherSessionId) == visibleEntities.end())
		{
			const uint32_t otherEntityId = mSessionToEntity[otherSessionId];
			const std::unique_ptr<Entity>& otherEntity = mEntitys[otherEntityId];

			S2C_DISAPPEAR_ENTITY protocol;
			protocol.ObjectId = entityId;

			auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_DISAPPEAR_ENTITY, protocol);
			mDirectMessageHandler(otherEntity->mSessionId, std::move(message));

			otherEntity->mMonitor.erase(entityId);
			otherSessionIdIter = entity->mViewer.erase(otherSessionIdIter);
		}
		else
		{
			++otherSessionIdIter;
		}
	}
}
```

### 유저의 정보를 나눠서 보내기

사람들이 많이 몰린곳에 진입 시 한번에 많은 양의 플레이어 정보를 얻는 것이 아닌 순차적으로 정보를 전달하여 안정적인 플레이가 가능하지 않을까 생각하여 </br>

서버에서 외형의 정보를 주면 클라이언트에서 이를 반영하고 서버에게 외형에 대한 정보를 모두 받았다고 전달하는 SYN, ACK처럼 동작하면 좋지 않을까 생각했습니다. </br>
또한 바로 바로 주고 받으면 한번에 주는 것보다는 아니겠지만 비슷하기 떄문에 100ms씩 텀을 주어 전달하여 트래픽을 줄여보고자 노력하였습니다. </br>

```
// 인포를 받았을 경우 처리 후 다시 전달
void Handle_PKT_S2C_ENTITY_INFO(const std::unique_ptr<World>& World, const S2C_ENTITY_INFO* Protocol)
{
	// 인포에 대한 처리
	//Protocol->Infos;

	if (Protocol->InfoNumber >= EEntityInfoPriority::ENTITY_INFO_MAX)
	{
		return;
	}

	C2S_ENTITY_INFO protocol;
	protocol.InfoNumber = Protocol->InfoNumber;
	protocol.ObjectId = Protocol->ObjectId;

	std::unique_ptr<Message> message = MessageSerializer::Serialize(EMessageId::PKT_C2S_ENTITY_INFO, protocol);
	World->mMessageHandler(std::move(message));
}
```

```
// 보내야 하는 정보가 Max이지 않는 이상 100ms 텀을 두고 잡 큐에 넣기
void Handle_PKT_C2S_ENTITY_INFO(const std::unique_ptr<World>& World, const std::shared_ptr<Session>& Session, const C2S_ENTITY_INFO* Protocol)
{

	uint8_t nextInfoNumber = Protocol->InfoNumber + 1;
	if (nextInfoNumber >= EEntityInfoPriority::ENTITY_INFO_MAX)
	{
		// 보내야 하는 데이터를 전부 보냄
		return;
	}

	// 100ms 딜레이하여 전송하기
	{
		uint32_t time = static_cast<uint32_t>(gTimeManager.GetServerTime());
		World->PushTask(time + 100, World, &World::NextEntityInfo, Session->GetSessionId(), nextInfoNumber, Protocol->ObjectId);
	}
}
```

```
// 다음 정보를 찾고 보내줄 수 있도록함함
void World::NextEntityInfo(const uint32_t SessionId, const uint8_t NextInfoNumber, const int32_t OtherEntityId)
{
	uint32_t entityId = mSessionToEntity[SessionId];
	const std::unique_ptr<Entity>& entity = mEntitys[entityId];

	auto otherEntityIter = entity->mMonitor.find(OtherEntityId);
	if (otherEntityIter == entity->mMonitor.end())
	{
		// 시야에서 사라짐
		return;
	}

	S2C_ENTITY_INFO protocol;
	protocol.InfoNumber = NextInfoNumber;
	protocol.ObjectId = OtherEntityId;
	//protocol.Infos;

	auto message = MessageSerializer::Serialize(EMessageId::PKT_S2C_ENTITY_INFO, protocol);
	mDirectMessageHandler(SessionId, std::move(message));
}
```

## Vcpkg
### SDL2 : 그래픽 및 입력
### Boost.asio : 네트워크

mkdir build <br>
cd build <br>

cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake <br>
cmake --build . --config Release <br>

## 조작
좌 클릭 : 이동 <br>

## 사용법
귀찮으시겠지만 다음 줄을 찾아 수정해주시면 됩니다... <br>
#define USE_AOI 1 = 이중 가시영역 <br>
#define USE_AOI 0 = 단일 가시영역 <br>

## 참고
[이득우의 꼭 배워야하는 게임 알고리즘](https://www.inflearn.com/course/%EA%B2%8C%EC%9E%84-%EC%95%8C%EA%B3%A0%EB%A6%AC%EC%A6%98?srsltid=AfmBOop6dMp3k7lA91OPR5NQBIGTTnWZBma8r3uTrY9XFidST7RZB5sU) - A*를 구현하는데 참고 하였습니다. <br>
[프라우드넷 가시거리] - 프라우드넷의 가시거리 예제를 참고 하였습니다. <br>
