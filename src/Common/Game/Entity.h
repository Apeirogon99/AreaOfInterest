#pragma once
#include "Common/Math/Vector2f.h"
#include "Common/AStar/Node.h"
#include "list"

class Entity
{
public:
	Entity(const uint32_t ObjectId, const Vector2f& Position);

public:
	void MoveTowardsNextPath(float DeltaTime);
	void PositionCorrection(float DeltaTime);
	Node* GetDestinationNode();

	void RecoveyIntervalMoveSync();

public:
	uint32_t mObjectId;

	Vector2f mPosition;
	Vector2f mVelocity;
	float mMoveSpeed;

	float mIntervalMoveSync;
	float mLastMoveSync;

	Vector2f mCorrectionStartPosition;
	Vector2f mCorrectionEndPosition;
	bool mIsCorrection;
	float mCorrectionTime;

	std::list<Node*> mPath;
};
