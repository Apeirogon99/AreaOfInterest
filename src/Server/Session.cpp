#include "Session.h"

Session::Session(boost::asio::ip::tcp::socket Socket, const uint32_t SessionId) :
    mSessionId(SessionId), mSocket(std::move(Socket)),
    mRecvBuffer(), mRecvBufferCount(0)
{
}

Session::~Session()
{
}

void Session::Start()
{
    ReadAsync();
}

void Session::Close()
{
    mSocket.close();
}

void Session::Write(std::unique_ptr<Message> Message)
{
    std::lock_guard<std::mutex> lock(mWriteMutex);
    bool isWrite = !mWriteQueue.empty();
    mWriteQueue.push(std::move(Message));

    if (!isWrite)
    {
        WriteAsync();
    }
}

void Session::ReadAsync()
{
    std::array<BYTE, 1024> data;

    mSocket.async_read_some(
        boost::asio::buffer(data, 1024),
        [this, &data](boost::system::error_code error, std::size_t length)
        {
            if (!error)
            {
                std::copy(data.begin(), data.begin() + length, mRecvBuffer.begin() + mRecvBufferCount);
                mRecvBufferCount += length;

                std::size_t processLength = 0;
                std::size_t dataLength = 0;
                std::size_t messageSize = 0;
                while (true)
                {
                    dataLength = length - processLength;

                    if (dataLength < sizeof(MessageHeader))
                        break;

                    MessageHeader* header = reinterpret_cast<MessageHeader*>(&mRecvBuffer[processLength]);
                    messageSize = static_cast<std::size_t>(header->mSize);
                    if (dataLength < messageSize)
                        break;

                    std::vector<BYTE> payload;
                    payload.resize(header->mSize);
                    std::copy(&mRecvBuffer[processLength], &mRecvBuffer[processLength + header->mSize], payload.begin());
                    std::unique_ptr<Message> message = std::make_unique<Message>(header->mId, header->mSize, payload);
                    mMessageHandler(shared_from_this(), std::move(message));

                    processLength += messageSize;
                }

                if (processLength > 0)
                {
                    std::size_t remainingLength = mRecvBufferCount - processLength;
                    if (remainingLength > 0)
                    {
                        std::memmove(&mRecvBuffer[0], &mRecvBuffer[processLength], remainingLength);
                    }
                    mRecvBufferCount = remainingLength;
                }

                ReadAsync();
            }
            else
            {
                mDisconnectHandler(shared_from_this(), error);
            }
        });
}

void Session::WriteAsync()
{
    const std::unique_ptr<Message>& message = mWriteQueue.front();

    mSocket.async_write_some(
        boost::asio::buffer(message->GetData()),
        [this](boost::system::error_code error, std::size_t length)
        {
            if (!error)
            {
                std::lock_guard<std::mutex> lock(mWriteMutex);
                mWriteQueue.pop();
                if (!mWriteQueue.empty())
                {
                    WriteAsync();
                }
            }
            else
            {
                mDisconnectHandler(shared_from_this(), error);
            }
        });
}
