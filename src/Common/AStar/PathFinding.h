#pragma once
#include "Common/AStar/Grid.h"
#include <memory>



class PathFinding
{
public:
	PathFinding();
	~PathFinding();

private:
	PathFinding(const PathFinding&) = delete;
	PathFinding operator=(const PathFinding&) = delete;

	PathFinding(PathFinding&&) = delete;
	PathFinding operator=(PathFinding&&) = delete;

public:
	std::list<Node*> FindPath(const std::unique_ptr<Grid>& Grid, Vector2f StartPosition, Vector2f TargetPosition);

private:
	std::list<Node*> RetracePath(Node* StartNode, Node* EndNode);
	int GetDistance(Node* Lhs, Node* Rhs);
};
