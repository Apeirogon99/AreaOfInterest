#include "Network.h"
#include <iostream>
#include <set>

Network::Network(const uint16_t Port) : 
    mIsRunning(false),
    mContext(), mAcceptor(mContext), mWorkGuard(boost::asio::make_work_guard(mContext)), mPort(Port)
{
}

Network::~Network()
{
}

bool Network::Initialize()
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), mPort);
    mAcceptor.open(endpoint.protocol());
    mAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    mAcceptor.bind(endpoint);
    mAcceptor.listen();

    mNetworkThread = std::thread([&]()
        {
            mContext.run();
        });

    mIsRunning = true;
	return true;
}

void Network::Destroy()
{
    mAcceptor.close();
    std::lock_guard<std::mutex> lock(mClientMutex);
    for (auto sessionIter : mClients)
    {
        const std::shared_ptr<Session>& session = sessionIter.second;
        if (session)
        {
            session->Close();
        }
    }
    mClients.clear();
}

void Network::Direct(const uint32_t SessionId, std::unique_ptr<Message> Message)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    auto sessionIter = mClients.find(SessionId);
    if (sessionIter != mClients.end())
    {
        sessionIter->second->Write(std::move(Message));
    }
}

void Network::Broadcast(const std::unordered_set<uint32_t>& SessionIds, std::unique_ptr<Message> Message)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    for (auto sessionId : SessionIds)
    {
        auto sessionIter = mClients.find(sessionId);
        if (sessionIter != mClients.end())
        {
            sessionIter->second->Write(std::move(Message));
        }
    }
}

void Network::AcceptAsync()
{
    mAcceptor.async_accept(
        [this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
        {
            if (!error)
            {
                static uint32_t SESSION_ID = 1;

                std::shared_ptr<Session> newClient = std::make_shared<Session>(std::move(socket), SESSION_ID);
                newClient->SetMessageHandler(mMessageHandler);
                newClient->SetConnectHandler(mConnectHandler);
                newClient->SetDisconnectHandler(mDisconnectHandler);

                mConnectHandler(newClient);
                mClients.insert({ SESSION_ID, newClient });
                newClient->Start();

                SESSION_ID++;
            }
            else
            {
                std::cerr << "Acception failed: " << error.message() << std::endl;
            }

            AcceptAsync();
        });
}
