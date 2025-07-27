#pragma once

#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Common/Game/Entity.h"
#include "Common/AStar/Node.h"

class NodeGroup
{
public:
	NodeGroup(const uint32_t GroupId, int GroupX, int GroupY) :
		mGroupId(GroupId), mGroupX(GroupX), mGroupY(GroupY)
	{
		
	}

public:
	std::unordered_set<uint32_t> mEntityIds;

	uint32_t mGroupId;
	int mGroupX;
	int mGroupY;
};

class AOI
{
public:
	AOI(const Vector2f& WorldSize, const float NodeSize, const uint8_t GroupScale);
	~AOI();

public:
	/// <summary>
	/// Entity를 그룹에 배치하기
	/// </summary>
	void PlaceEntityInGroup(const std::map<uint32_t, std::unique_ptr<Entity>>& Entitys);

	/// <summary>
	/// 하나의 가시거리를 업데이트
	/// </summary>
	void UpdateSingleVisibleDistance();

	/// <summary>
	/// 여러개의 가시거리를 업데이트
	/// </summary>
	void UpdateRangeVisibleDistance();

public:
	std::list<NodeGroup*> GetNodeGroupInRange(const Vector2f& Position, const float Range);

public:
	std::unordered_map<uint32_t, std::unique_ptr<NodeGroup>> mGroups;

	Vector2f mWorldSize;
	float mGroupSize;
	int mGroupCountX;
	int mGroupCountY;
};