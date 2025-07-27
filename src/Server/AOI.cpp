#include "AOI.h"

#include <algorithm>
#include <cmath>

AOI::AOI(const Vector2f& WorldSize, const float NodeSize, const uint8_t GroupScale) : 
	mWorldSize(WorldSize), mGroupSize(NodeSize * GroupScale)
{
	mGroupCountX = static_cast<int>(lround(mWorldSize.x / mGroupSize));
	mGroupCountY = static_cast<int>(lround(mWorldSize.y / mGroupSize));

	for (int y = 0; y < mGroupCountY; ++y)
	{
		for (int x = 0; x < mGroupCountX; ++x)
		{
			const uint32_t groupId = y * mGroupCountX + x;
			mGroups.insert({ groupId, std::make_unique<NodeGroup>(groupId, x, y)});
		}
	}
}

AOI::~AOI()
{
}

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
			// ±âÁ¸ ±×·ìÀÌ ÀÖ´Ù¸é Å»Åð
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

std::list<NodeGroup*> AOI::GetNodeGroupInRange(const Vector2f& Position, const float Range)
{
	std::list<NodeGroup*> groups;

	float minX = Position.x - Range;
	float maxX = Position.x + Range;
	float minY = Position.y - Range;
	float maxY = Position.y + Range;

	int minGroupX = static_cast<int>(floorf(minX / mGroupSize));
	int maxGroupX = static_cast<int>(floorf(maxX / mGroupSize));
	int minGroupY = static_cast<int>(floorf(minY / mGroupSize));
	int maxGroupY = static_cast<int>(floorf(maxY / mGroupSize));

	minGroupX = std::max(0, minGroupX);
	maxGroupX = std::min(mGroupCountX - 1, maxGroupX);
	minGroupY = std::max(0, minGroupY);
	maxGroupY = std::min(mGroupCountY - 1, maxGroupY);

	for (int groupY = minGroupY; groupY <= maxGroupY; ++groupY)
	{
		for (int groupX = minGroupX; groupX <= maxGroupX; ++groupX)
		{
			int groupId = groupY * mGroupCountX + groupX;
			if (groupId >= 0 && groupId < mGroups.size())
			{
				groups.push_back(mGroups[groupId].get());
			}
		}
	}

	return groups;
}
