#include "Common/Protocol/Protocol.h"

const char* MessageIdToString(unsigned __int16 Id)
{
	switch (static_cast<EMessageId>(Id))
	{
	case PKT_S2C_SPAWN_ENTITY:
		return "SPAWN_ENTITY";
		break;
	case PKT_S2C_APPEAR_ENTITY:
		return "APPEAR_ENTITY";
		break;
	case PKT_S2C_DISAPPEAR_ENTITY:
		return "DISAPPEAR_ENTITY";
		break;
	case PKT_S2C_ENTITY_INFO:
		return "ENTITY_INFO";
		break;
	case PKT_S2C_PATH_FINDING:
		return "PATH_FINDING";
		break;
	case PKT_S2C_POSITION_SYNC:
		return "POSITION_SYNC";
		break;
	default:
		break;
	}
    return nullptr;
}

const size_t MessageIdToByte(unsigned __int16 Id)
{
	switch (static_cast<EMessageId>(Id))
	{
	case PKT_S2C_SPAWN_ENTITY:
		return sizeof(S2C_SPAWN_ENTITY);
		break;
	case PKT_S2C_APPEAR_ENTITY:
		return sizeof(S2C_APPEAR_ENTITY);
		break;
	case PKT_S2C_DISAPPEAR_ENTITY:
		return sizeof(S2C_DISAPPEAR_ENTITY);
		break;
	case PKT_S2C_ENTITY_INFO:
		return sizeof(S2C_ENTITY_INFO);
		break;
	case PKT_S2C_PATH_FINDING:
		return sizeof(S2C_PATH_FINDING);
		break;
	case PKT_S2C_POSITION_SYNC:
		return sizeof(S2C_POSITION);
		break;
	default:
		break;
	}
	return 0;
}
