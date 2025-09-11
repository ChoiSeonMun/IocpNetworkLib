// TestServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <csmnet/Server.h>
#include <csmnet/ServerSession.h>
#include <csmnet/util/SpdLogger.h>

#include <iostream>
#include <print>

#pragma comment(lib, "IocpNetworkLib.lib")

class MyServerSession : public csmnet::ServerSession
{
public:
    using ServerSession::ServerSession;

    void OnConnected() override
    {
        auto remote = GetRemoteEndpoint();
        std::println("클라이언트 접속: {}:{}", remote.GetIp(), remote.GetPort());
    }

    void OnDisconnected() override
    {
        auto remote = GetRemoteEndpoint();
        std::println("클라이언트 접속 해제: {}:{}", remote.GetIp(), remote.GetPort());
    }

    void OnRecv(std::span<const std::byte> message) override
    {
        auto remote = GetRemoteEndpoint();
        _logger.Info(format("MyServerSession::OnRecv - Received : {} From {}:{}",
            reinterpret_cast<const char*>(message.data()),
            remote.GetIp(),
            remote.GetPort()));

        auto sendBuffer = GetSendBuffer();
        if (sendBuffer.EnsureWritable(message.size()) == false)
        {
            _logger.Error("MyServerSession::OnRecv - 송신 버퍼의 크기가 부족합니다.");
            Disconnect();
            return;
        }
        auto writableSpan = sendBuffer.Poke(message.size());
        std::ranges::copy(message, writableSpan.data());
        sendBuffer.Advance(message.size());

        if (auto result = Send(sendBuffer); !result)
        {
            _logger.Error(format("MyServerSession::OnRecv - fail to send message : [{}] {}",
                result.error().value(),
                result.error().message()));
            Disconnect();
            return;
        }

        _logger.Info("MyServerSession::OnRecv - Echoed");
    }
};

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    csmnet::ServerConfig config;
    config.MaxSessionCount = 1;
    config.Port = 12345;
    config.IoThreadCount = 4;
    config.SessionCleanIntervalSec = 5;

    csmnet::util::SpdConsoleLogger logger; 
    csmnet::Server<MyServerSession>::SessionFactory sessionFactory = [&logger]() -> MyServerSession
        {
            constexpr size_t kDefaultRecvBufferSize = 0x10000; // 64KB
            constexpr size_t kDefaultSendBufferSize = 0x10000; // 64KB

            MyServerSession session(logger);
            session.SetRecvBufferSize(kDefaultRecvBufferSize);
            session.SetSendBufferSize(kDefaultSendBufferSize);

            return session;
        };

    csmnet::Server<MyServerSession> server(logger, std::move(config), std::move(sessionFactory), {});
    if (auto result = server.Open(); !result)
    {
        println("[Error: {}] Failed to open server : {}", static_cast<int>(result.error().value()), result.error().message());
        return 1;
    }

    std::println("아무 키나 누르세요...");
    char input;
    std::cin >> input;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file