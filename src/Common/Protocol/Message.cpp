#include "Common/Protocol/Message.h"

Message::Message(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data) : mHeader({ Id, Size }), mData(Data)
{

}
