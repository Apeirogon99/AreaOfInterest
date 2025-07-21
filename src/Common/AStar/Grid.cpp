#include "Common/AStar/Grid.h"
#include <cmath>
#include <algorithm>

Grid::Grid(const Vector2f& GridSize, float NodeSize) : mNumberOfGrids(GridSize), mNodeSize(NodeSize), mNodeHalfSize(NodeSize * 0.5f)
{
	mGridSizeX = static_cast<int>(lround(mNumberOfGrids.x / mNodeSize));
	mGridSizeY = static_cast<int>(lround(mNumberOfGrids.y / mNodeSize));

	mGrid.resize(mGridSizeY);
	for (int y = 0; y < mGridSizeY; ++y)
	{
		mGrid[y].resize(mGridSizeX);
		for (int x = 0; x < mGridSizeX; ++x)
		{
			Vector2f nodePosition = Vector2f(x * mNodeSize + mNodeHalfSize, y * mNodeSize + mNodeHalfSize);
			mGrid[y][x] = std::make_unique<Node>(true, nodePosition, x, y);
		}
	}
}

Grid::~Grid()
{
	for (int x = 0; x < mGrid.size(); ++x)
	{
		for (int y = 0; y < mGrid[0].size(); ++y)
		{
			if (mGrid[y][x] != nullptr)
			{		  
				mGrid[y][x].reset();
				mGrid[y][x] = nullptr;
			}
		}
	}
}

std::list<Node*> Grid::GetNeighbours(Node* TargetNode)
{
	std::list<Node*> neighbours;
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			if (x == 0 && y == 0)
			{
				continue;
			}

			int checkX = TargetNode->mGridX + x;
			int checkY = TargetNode->mGridY + y;

			if (checkX >= 0 && checkX < mGridSizeX && checkY >= 0 && checkY < mGridSizeY)
			{
				neighbours.push_back(mGrid[checkY][checkX].get());
			}
		}
	}
	return neighbours;
}

Node* Grid::GetNodeFromPosition(Vector2f Position)
{
	float percentX = Position.x / mNumberOfGrids.x;
	float percentY = Position.y / mNumberOfGrids.y;
	percentX = std::clamp(percentX, 0.0f, 1.0f);
	percentY = std::clamp(percentY, 0.0f, 1.0f);

	int x = static_cast<int>(lround((mGridSizeX - 1) * percentX));
	int y = static_cast<int>(lround((mGridSizeY - 1) * percentY));
	return mGrid[y][x].get();
}

GridDirection Grid::GetGridDirection(Node* From, Node* To)
{
	int dirX = To->mGridX - From->mGridX;
	int dirY = To->mGridY - From->mGridY;

	if(dirX == 0 && dirY == 0) return GridDirection::None;

	int noDirX = (dirX > 0) ? 1 : (dirX < 0) ? -1 : 0;
	int noDirY = (dirY > 0) ? 1 : (dirY < 0) ? -1 : 0;

	if (noDirX == +0 && noDirY == -1)	return GridDirection::North;
	if (noDirX == +1 && noDirY == -1)	return GridDirection::NorthEast;
	if (noDirX == +1 && noDirY == +0)	return GridDirection::East;
	if (noDirX == +1 && noDirY == +1)	return GridDirection::SouthEast;
	if (noDirX == +0 && noDirY == +1)	return GridDirection::South;
	if (noDirX == -1 && noDirY == +1)	return GridDirection::SouthWest;
	if (noDirX == -1 && noDirY == +0)	return GridDirection::West;
	if (noDirX == -1 && noDirY == -1)	return GridDirection::NorthWest;

	return GridDirection::None;
}

int Grid::GetGridDirectionAngle(GridDirection Direction)
{
	switch (Direction)
	{
		case GridDirection::North:		return 0;
		case GridDirection::NorthEast:	return 45;
		case GridDirection::East:		return 90;
		case GridDirection::SouthEast:	return 135;
		case GridDirection::South:		return 180;
		case GridDirection::SouthWest:	return 225;
		case GridDirection::West:		return 270;
		case GridDirection::NorthWest:	return 315;
		default: return 0;
	}
}

bool Grid::IsAngleExceeded(const GridDirection From, const GridDirection To, const int Angle)
{
	int fromAngle = GetGridDirectionAngle(From);
	int toAngle = GetGridDirectionAngle(To);

	int angleDiff = std::abs(fromAngle - toAngle);
	if (angleDiff > 180)
	{
		angleDiff = 360 - angleDiff;
	}

	return angleDiff >= Angle;
}