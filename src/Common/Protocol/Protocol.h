#pragma once
#include <vector>
#include "Common/Math/Vector2f_NetQuantize.h"

enum EMessageId : unsigned __int16
{
	PKT_S2C_SPAWN_ENTITY,

	PKT_S2C_APPEAR_ENTITY,
	PKT_S2C_DISAPPEAR_ENTITY,

	PKT_C2S_ENTITY_INFO,
	PKT_S2C_ENTITY_INFO,
	
	PKT_C2S_PATH_FINDING,
	PKT_S2C_PATH_FINDING,
	
	PKT_S2C_POSITION_SYNC,
};

enum EEntityInfoPriority : unsigned __int8
{
	ENTITY_INFO_APPEAR,
	ENTITY_INFO_APPEARANCE,
	ENTITY_INFO_EQUIPMENT,
	ENTITY_INFO_STATE,
	ENTITY_INFO_MAX,
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

struct C2S_ENTITY_INFO
{
	uint8_t InfoNumber;
	int32_t ObjectId;
};

struct S2C_ENTITY_INFO
{
	uint8_t InfoNumber;
	int32_t ObjectId;
	int32_t Infos[10];	// 인포마다 동적으로 다르겠지만 테스트 용이기에 고정
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