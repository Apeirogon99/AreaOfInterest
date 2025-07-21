#pragma once
#include <vector>
#include "Common/Math/Vector2f_NetQuantize.h"

enum class EMessageId : unsigned __int16
{
	None,
	PKT_S2C_SPAWN_ENTITY,
	PKT_S2C_LOAD_MAP,

	PKT_C2S_PATH_FINDING,
	PKT_S2C_PATH_FINDING,
	PKT_S2C_PATH_REFINDING,

	PKT_S2C_POSITION_SYNC,

	PKT_C2S_TOGGLE_WALKABLE,
	PKT_S2C_TOGGLE_WALKABLE,
};

struct Waypoint
{
	int GridX;
	int GridY;
};

struct S2C_SPAWN_ENTITY
{
	int ObjectId;
	int GridX;
	int GridY;
};

struct S2C_LOAD_MAP
{
	
};

struct C2S_PATH_FINDING
{
	unsigned __int32 TimeStamp;
	int DestGridX;
	int DestGridY;
};

struct S2C_PATH_FINDING
{
	unsigned __int32 TimeStamp;
	int ObjectId;
	float EntityPositionX;
	float EntityPositionY;

	int PathCount;
	Waypoint Path[50];
};

struct S2C_PATH_REFINDING
{
	unsigned __int32 TimeStamp;
	int ObjectId;

	int PathCount;
	Waypoint Path[50];
};

struct S2C_POSITION
{
	unsigned __int32 TimeStamp;
	int ObjectId;
	float EntityPositionX;
	float EntityPositionY;
	int NextGridX;
	int NextGridY;
};

struct C2S_TOGGLE_WALKABLE
{
	int GridX;
	int GridY;
};

struct S2C_TOGGLE_WALKABLE
{
	int GridX;
	int GridY;
	bool IsWalkable;
};