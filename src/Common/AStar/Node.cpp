#include "Common/AStar/Node.h"

Node::Node(bool Walkable, const Vector2f& Position, int GridX, int GridY)
	: mIsWalkable(Walkable), mPosition(Position), mGridX(GridX), mGridY(GridY), mActualCost(0), mHeuristicCost(0), mParent(nullptr)
{
}

Node::~Node()
{
	mParent = nullptr;
}


bool CompareNodePosition(const Node* lhs, const Node* rhs)
{
	return (lhs->mGridX == rhs->mGridX && lhs->mGridY == rhs->mGridY);
}

bool CompareNodePositions(const std::list<Node*>& lList, const std::list<Node*>& rList)
{
	return lList.size() == rList.size() &&
		std::equal(lList.begin(), lList.end(), rList.begin(), CompareNodePosition);
}