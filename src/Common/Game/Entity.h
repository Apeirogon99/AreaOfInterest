#pragma once
#include "Common/Math/Vector2f.h"
#include "Common/AStar/Node.h"
#include <list>
#include <unordered_set>

class Entity
{
public:
	Entity(const uint32_t ObjectId, const Vector2f& Position);
	virtual ~Entity();

public:
	// 공용
	void MoveTowardsNextPath(float DeltaTime);

	// 서버
	void RecoveyIntervalMoveSync();

	// 클라
	void PositionCorrection(float DeltaTime);

public:
	// 공용
	uint32_t mObjectId;
	Vector2f mPosition;
	Vector2f mVelocity;
	float mMoveSpeed;
	std::list<Node*> mPath;

	// 서버
	bool mIsAI;
	float mIntervalMoveSync;
	float mLastMoveSync;

	uint32_t mSessionId;
	std::unordered_set<uint32_t> mViewer;	// 보고 있는 (세션 ID)
	std::unordered_set<uint32_t> mMonitor;	// 보여지고 있는 (Entity ID)

	// 클라이언트
	Vector2f mCorrectionStartPosition;
	Vector2f mCorrectionEndPosition;
	bool mIsCorrection;
	float mCorrectionTime;
};