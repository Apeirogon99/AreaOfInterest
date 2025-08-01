#include "Common/Game/Entity.h"
#include <algorithm>

Entity::Entity(const uint32_t ObjectId, const Vector2f& Position) :
	mObjectId(ObjectId), mPosition(Position), mVelocity(), mMoveSpeed(50.0f),
	mIsAI(false), mLastNearSync(0.0f), mSingleLastSync(0.0f), mLastFarSync(0.0f), mNodeGroupId(UINT32_MAX), mSessionId(0),
	mCorrectionStartPosition(), mCorrectionEndPosition(), mIsCorrection(false), mCorrectionTime(0.0f)
{

}

Entity::~Entity()
{
}

void Entity::MoveTowardsNextPath(float DeltaTime)
{
	if (mPath.size() <= 0) return;

	float moveableDistance = mMoveSpeed * DeltaTime; // 이동할 수 있는 거리

	while (moveableDistance > 0.0f && !mPath.empty())
	{
		Node* destNode = mPath.front();
		Vector2f destPosition = destNode->mPosition;
		Vector2f direction = destPosition - mPosition;
		float distanceToDest = direction.magnitude(); // 목적지까지의 거리
		
		if (moveableDistance >= distanceToDest)
		{
			mPosition = destPosition;
			moveableDistance -= distanceToDest;
			mPath.pop_front();

			if (!mPath.empty())
			{
				Vector2f nextDirection = (mPath.front()->mPosition - mPosition).normalized();
				mVelocity = nextDirection * mMoveSpeed;
			}
			else
			{
				mVelocity = { 0.0f, 0.0f };
			}
		}
		else
		{
			// 이동할 수 있는 만큼만 이동
			Vector2f unitDirection = direction.normalized();
			mPosition += unitDirection * moveableDistance;
			mVelocity = unitDirection * mMoveSpeed;
			break;
		}

	}

	//printf("[%f][%f]\n", mPosition.y, mPosition.x);
}

void Entity::PositionCorrection(float DeltaTime)
{
	const float CorrectionDuration = 0.03f;

	mCorrectionTime += DeltaTime;
	float t = std::fmin(mCorrectionTime / CorrectionDuration, 1.0f);

	if (t >= 1.0f)
	{
		mPosition = mCorrectionEndPosition;
		mIsCorrection = false;
	}
	else
	{
		float smoothT = t * t * (3.0f - 2.0f * t);
		mPosition = Lerp(mCorrectionStartPosition, mCorrectionEndPosition, smoothT);
	}
}