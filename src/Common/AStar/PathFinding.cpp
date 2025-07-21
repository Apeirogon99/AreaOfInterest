#include "Common/AStar/PathFinding.h"
#include <cmath>
#include <list>
#include <set>
#include <algorithm>

PathFinding::PathFinding()
{
}

PathFinding::~PathFinding()
{

}

std::list<Node*> PathFinding::FindPath(const std::unique_ptr<Grid>& Grid, Vector2f StartPosition, Vector2f TargetPosition)
{
	std::list<Node*> path;

	Node* startNode = Grid->GetNodeFromPosition(StartPosition);
	Node* targetNode = Grid->GetNodeFromPosition(TargetPosition);

	std::list<Node*> openSet;
	std::set<Node*> closedSet;

	openSet.push_back(startNode);
	while (openSet.size() > 0)
	{
		// 가장 낮은 값을 가진 노드를 선택한다.
		Node* currentNode = *openSet.begin();
		for (auto iter = openSet.begin(); iter != openSet.end(); ++iter)
		{
			Node* node = *iter;
			if (node->GetTotalCost() < currentNode->GetTotalCost() ||
				(node->GetTotalCost() == currentNode->GetTotalCost() &&
					node->mHeuristicCost < currentNode->mHeuristicCost))
			{
				currentNode = node;
			}
		}

		// 가장 낮은 값을 가진 노드가 종착노드면 탐색을 종료한다.
		if (currentNode == targetNode)
		{
			path = RetracePath(startNode, targetNode);
			break;
		}

		// 현재 노드를 오픈 셋에서 빼서 클로즈드 셋으로 이동한다.
		openSet.remove(currentNode);
		closedSet.insert(currentNode);

		// 이웃노드를 가져와서 값을 계산한 후 오픈 셋에 추가한다.
		for(Node* neighbourNode : Grid->GetNeighbours(currentNode))
		{
			if (!neighbourNode->mIsWalkable || closedSet.contains(neighbourNode))
			{
				continue;
			}

			int g = currentNode->mActualCost + GetDistance(currentNode, neighbourNode);
			int h = GetDistance(neighbourNode, targetNode);
			int f = g + h;

			// 오픈 셋에 이미 중복 노드가 있는 경우 값이 작은 쪽으로 변경한다.
			if (std::find(openSet.begin(), openSet.end(), neighbourNode) == openSet.end())
			{
				neighbourNode->mActualCost = g;
				neighbourNode->mHeuristicCost = h;
				neighbourNode->mParent = currentNode;
				openSet.push_back(neighbourNode);
			}
			else
			{
				if (neighbourNode->GetTotalCost() > f)
				{
					neighbourNode->mActualCost = g;
					neighbourNode->mParent = currentNode;
				}
			}
		}
	}

	return path;
}



std::list<Node*> PathFinding::RetracePath(Node* StartNode, Node* EndNode)
{
	std::list<Node*> path;

	Node* currentNode = EndNode;
	GridDirection lastDirection = GridDirection::None;

	while (currentNode != StartNode) 
	{
		GridDirection currentDirection = Grid::GetGridDirection(currentNode, currentNode->mParent);

		if (lastDirection != currentDirection)
		{
			path.push_back(currentNode);
			lastDirection = currentDirection;
		}
		currentNode = currentNode->mParent;
	}

	path.reverse();
	return path;
}

int PathFinding::GetDistance(Node* Lhs, Node* Rhs)
{
	int dstX = abs(Lhs->mGridX - Rhs->mGridX);
	int dstY = abs(Lhs->mGridY - Rhs->mGridY);
	
	if (dstX > dstY)
	{
		return 14 * dstY + 10 * (dstX - dstY);
	}
	return 14 * dstX + 10 * (dstY - dstX);
}
