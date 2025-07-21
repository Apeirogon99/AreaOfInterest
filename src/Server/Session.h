#pragma once
#include <memory>
#include <queue>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "Common/Protocol/Message.h"

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket Socket, const uint32_t SessionId);
    ~Session();

public:
    void Start();
    void Close();

public:
    void Write(std::unique_ptr<Message> Message);

public:
    boost::asio::ip::tcp::socket& GetSocket() { return mSocket; }
    inline uint32_t GetSessionId() const { return mSessionId; }

public:
    void SetMessageHandler(std::function<void(const std::shared_ptr<Session>&, std::unique_ptr<Message>)> Handler)
    {
        mMessageHandler = Handler;
    }

    void SetConnectHandler(std::function<void(const std::shared_ptr<Session>&)> Handler)
    {
        mConnectHandler = Handler;
    }

    void SetDisconnectHandler(std::function<void(const std::shared_ptr<Session>&, boost::system::error_code&)> Handler)
    {
        mDisconnectHandler = Handler;
    }

private:
    void ReadAsync();
    void WriteAsync();

private:
    uint32_t mSessionId;
    boost::asio::ip::tcp::socket mSocket;

    // Read, Write
    std::array<BYTE, 8192> mRecvBuffer;
    std::size_t mRecvBufferCount;
    std::queue<std::unique_ptr<Message>> mWriteQueue;
    std::mutex mWriteMutex;

    // Handler
    std::function<void(const std::shared_ptr<Session>&, std::unique_ptr<Message>)> mMessageHandler;
    std::function<void(const std::shared_ptr<Session>&)> mConnectHandler;
    std::function<void(const std::shared_ptr<Session>&, boost::system::error_code&)> mDisconnectHandler;
};
