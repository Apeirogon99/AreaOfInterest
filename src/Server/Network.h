#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <queue>
#include <unordered_set>

#include "Session.h"
#include "Common/Protocol/Message.h"

class Network
{
public:
    Network(const uint16_t Port);
    ~Network();

public:
    bool Initialize();
    void Destroy();

public:
    void AcceptAsync();

public:
    void Broadcast(std::unique_ptr<Message> Message);

public:
    inline bool IsRunning() const { return mIsRunning; }

public:
    void SetMessageHandler(std::function<void(std::shared_ptr<Session>, std::unique_ptr<Message>)> Handler)
    {
        mMessageHandler = Handler;
    }

    void SetConnectHandler(std::function<void(std::shared_ptr<Session>)> Handler)
    {
        mConnectHandler = Handler;
    }

    void SetDisconnectHandler(std::function<void(std::shared_ptr<Session>, boost::system::error_code&)> Handler)
    {
        mDisconnectHandler = Handler;
    }

private:
    bool mIsRunning;

    std::thread mNetworkThread;
    boost::asio::io_context mContext;
    boost::asio::ip::tcp::acceptor mAcceptor;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;

    std::string mHost;
    uint16_t mPort;

    // Session
    std::vector<std::shared_ptr<Session>> mClients;
    std::mutex mClientMutex;

    // Handler
    std::function<void(std::shared_ptr<Session>, std::unique_ptr<Message>)> mMessageHandler;
    std::function<void(std::shared_ptr<Session>)> mConnectHandler;
    std::function<void(std::shared_ptr<Session>, boost::system::error_code&)> mDisconnectHandler;
};
