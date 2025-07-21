#include "Network.h"
#include <iostream>

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
    for (std::shared_ptr<Session>& client : mClients)
    {
        client->Close();
    }
    mClients.clear();
}

void Network::Broadcast(std::unique_ptr<Message> Message)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    for (const std::shared_ptr<Session>& client : mClients)
    {
        client->Write(std::move(Message));
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

                std::shared_ptr<Session> newClient = std::make_shared<Session>(std::move(socket), SESSION_ID++);
                newClient->SetMessageHandler(mMessageHandler);
                newClient->SetConnectHandler(mConnectHandler);
                newClient->SetDisconnectHandler(mDisconnectHandler);

                mConnectHandler(newClient);
                mClients.emplace_back(newClient);
                newClient->Start();
            }
            else
            {
                std::cerr << "Acception failed: " << error.message() << std::endl;
            }

            AcceptAsync();
        });
}
