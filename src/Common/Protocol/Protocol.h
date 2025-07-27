#pragma once
#include <vector>
#include "Common/Math/Vector2f_NetQuantize.h"

enum EMessageId : unsigned __int16
{
	None,
	PKT_S2C_SPAWN_ENTITY,

	PKT_S2C_APPEAR_ENTITY,
	PKT_S2C_DISAPPEAR_ENTITY,
	
	PKT_C2S_PATH_FINDING,
	PKT_S2C_PATH_FINDING,
	
	PKT_S2C_POSITION_SYNC,
};

const char* MessageIdToString(unsigned __int16 Id);

struct GridPoint
{
	int32_t		X;
	int32_t		Y;
};

struct S2C_SPAWN_ENTITY
{
	int32_t		ObjectId;
	GridPoint	SpawnGridPoint;
};

struct S2C_APPEAR_ENTITY
{
	uint32_t	TimeStamp;
	int32_t		ObjectId;
	Vector2f	EntityPosition;
};

struct S2C_DISAPPEAR_ENTITY
{
	int32_t		ObjectId;
};

struct C2S_PATH_FINDING
{
	uint32_t	TimeStamp;
	GridPoint	DestGridPoint;
};

struct S2C_PATH_FINDING
{
	uint32_t	TimeStamp;
	int32_t		ObjectId;
	Vector2f	EntityPosition;

	int32_t		PathCount;
	GridPoint	Path[50];
};

struct S2C_POSITION
{
	uint32_t	TimeStamp;
	int32_t		ObjectId;
	Vector2f	EntityPosition;
	GridPoint	DestGridPoint;
};