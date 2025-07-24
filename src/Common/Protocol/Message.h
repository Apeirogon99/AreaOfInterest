#pragma once
#include <vector>
#include <memory>

using BYTE = unsigned char;

struct MessageHeader
{
    uint16_t mId;
    uint16_t mSize;
};

class Message
{
public:
    explicit Message() = default;
    explicit Message(uint16_t Id, uint16_t Size, const std::vector<BYTE>& Data);

public:
    uint16_t GetId() const { return mHeader.mId; }
    uint16_t GetSize() const { return mHeader.mSize; }
    const std::vector<BYTE>& GetData() const { return mData; }

private:
    MessageHeader       mHeader;
    std::vector<BYTE>   mData;
};

class MessageSerializer
{
public:
    template<typename T>
    static std::unique_ptr<Message> Serialize(uint16_t Id, const T& Object)
    {
        const size_t dataSize = sizeof(Object);
        const size_t messageSize = dataSize + sizeof(MessageHeader);

        std::vector<BYTE> buffer;
        buffer.reserve(messageSize);

        MessageHeader messageHeader = { Id, static_cast<uint16_t>(messageSize) };

        const BYTE* header = reinterpret_cast<const BYTE*>(&messageHeader);
        const BYTE* data = reinterpret_cast<const BYTE*>(&Object);

        buffer.insert(buffer.end(), header, header + sizeof(MessageHeader));
        buffer.insert(buffer.end(), data, data + sizeof(Object));

        return std::make_unique<Message>(Id, static_cast<uint16_t>(messageSize), buffer);
    }

    template<typename T>
    static const T* Deserialize(const std::vector<BYTE>& Data, uint16_t Size)
    {
        if (Data.size() != Size - sizeof(MessageHeader))
        {
            return nullptr;
        }
        return reinterpret_cast<const T*>(Data.data());
    }
};