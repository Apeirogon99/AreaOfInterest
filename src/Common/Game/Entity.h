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
	// ����
	void MoveTowardsNextPath(float DeltaTime);

	// ����
	void RecoveyIntervalMoveSync();

	// Ŭ��
	void PositionCorrection(float DeltaTime);

public:
	// ����
	uint32_t mObjectId;
	Vector2f mPosition;
	Vector2f mVelocity;
	float mMoveSpeed;
	std::list<Node*> mPath;

	// ����
	bool mIsAI;
	float mIntervalMoveSync;
	float mLastMoveSync;

	uint32_t mSessionId;
	std::unordered_set<uint32_t> mViewer;	// ���� �ִ� (���� ID)
	std::unordered_set<uint32_t> mMonitor;	// �������� �ִ� (Entity ID)

	// Ŭ���̾�Ʈ
	Vector2f mCorrectionStartPosition;
	Vector2f mCorrectionEndPosition;
	bool mIsCorrection;
	float mCorrectionTime;
};