#include "Network.h"
#include <iostream>
#include <cmath>

std::unordered_map<uint16_t, size_t> gProcessMessageCount;

Network::Network(const std::string& Host, const uint16_t Port) : 
    mIsConnected(false),
    mContext(), mSocket(mContext), mWorkGuard(boost::asio::make_work_guard(mContext)), mHost(Host), mPort(Port),
    mRecvBuffer(), mRecvBufferCount(0)
{
    
}

Network::~Network()
{
}

bool Network::Initialize()
{
    mNetworkThread = std::thread([&]()
        {
            mContext.run();
        });

	return true;
}

void Network::Destroy()
{
    Disconnect();
}

void Network::Connect()
{
    boost::asio::ip::tcp::resolver resolver(mContext);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(mHost, std::to_string(mPort));

    boost::asio::async_connect(
        mSocket,
        endpoints,
        [&](boost::system::error_code error, boost::asio::ip::tcp::endpoint ep)
        {
            if (!error)
            {
                mConnectHandler();
                mIsConnected = true;
                ReadAsync();
            }
            else
            {
                std::cerr << "Connection failed: " << error.message() << std::endl;
            }
        });

}

void Network::Disconnect()
{
    mNetworkThread.join();

    mSocket.close();
    mIsConnected = false;
}

void Network::Write(std::unique_ptr<Message> Message)
{
    if (!IsConnected()) return;

    std::lock_guard<std::mutex> lock(mWriteMutex);
    bool isWrite = !mWriteQueue.empty();
    mWriteQueue.push(std::move(Message));

    if (!isWrite)
    {
        WriteAsync();
    }
}

void Network::PollMessage()
{
    mProcessBatch.clear();
    mProcessBatch.reserve(MAX_BATCH_SIZE);
    {
        std::lock_guard<std::mutex> lock(mRecvMutex);

        auto iter = mRecvMessages.begin();
        size_t batchCount = 0;

        while (iter != mRecvMessages.end() && batchCount < MAX_BATCH_SIZE)
        {
            auto message = mRecvMessages.extract(iter++);
            mProcessBatch.push_back(std::move(message.value()));
            batchCount++;
        }
    }

    for (auto& message : mProcessBatch)
    {
        gProcessMessageCount[message->GetId()] += 1;
        mMessageHandler(std::move(message));
    }
}

void Network::ReadAsync()
{
    if (mRecvBufferCount >= MAX_BUFFER_SIZE)
    {
        boost::system::error_code code = boost::asio::error::no_buffer_space;
        mDisconnectHandler(code);
        return;
    }

    int canReadByte = std::min(MAX_BUFFER_SIZE - static_cast<int>(mRecvBufferCount), READ_CHUNK_SIZE);
    if (canReadByte <= 0)
    {
        boost::system::error_code code = boost::asio::error::no_buffer_space;
        mDisconnectHandler(code);
        return;
    }

    mSocket.async_read_some(
        boost::asio::buffer(mReadBuffer, canReadByte),
        [this](boost::system::error_code error, std::size_t length)
        {
            if (!error)
            {
                mTempMessages.clear();

                std::copy(mReadBuffer.begin(), mReadBuffer.begin() + length, mRecvBuffer.begin() + mRecvBufferCount);
                mRecvBufferCount += length;

                std::size_t processLength = 0;
                std::size_t dataLength = 0;
                std::size_t messageSize = 0;
                while (true)
                {
                    dataLength = mRecvBufferCount - processLength;

                    if (dataLength < sizeof(MessageHeader))
                        break;

                    MessageHeader* header = reinterpret_cast<MessageHeader*>(&mRecvBuffer[processLength]);
                    messageSize = static_cast<std::size_t>(header->mSize);
                    if (dataLength < messageSize)
                        break;

                    std::vector<BYTE> payload;
                    payload.reserve(header->mSize - sizeof(MessageHeader));

                    std::size_t payloadSize = header->mSize - sizeof(MessageHeader);
                    if (payloadSize > 0)
                    {
                        payload.assign(
                            &mRecvBuffer[processLength + sizeof(MessageHeader)],
                            &mRecvBuffer[processLength + header->mSize]);
                    }

                    mTempMessages.insert(std::make_unique<Message>(header->mId, header->mSize, payload));

                    processLength += messageSize;
                }

                if (processLength > 0)
                {
                    {
                        std::lock_guard<std::mutex> lock(mRecvMutex);
                        mRecvMessages.merge(mTempMessages);
                    }

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
                // boost::asio::error::eof
                mDisconnectHandler(error);
            }
        });
}

void Network::WriteAsync()
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
                mDisconnectHandler(error);
            }
        });
}
