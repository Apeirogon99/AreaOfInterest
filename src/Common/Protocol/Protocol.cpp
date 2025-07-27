#include "Common/Protocol/Protocol.h"

const char* MessageIdToString(unsigned __int16 Id)
{
	switch (static_cast<EMessageId>(Id))
	{
	case None:
		break;
	case PKT_S2C_SPAWN_ENTITY:
		return "SPAWN_ENTITY";
		break;
	case PKT_S2C_APPEAR_ENTITY:
		return "APPEAR_ENTITY";
		break;
	case PKT_S2C_DISAPPEAR_ENTITY:
		return "DISAPPEAR_ENTITY";
		break;
	case PKT_C2S_PATH_FINDING:
		return "PATH_FINDING";
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
