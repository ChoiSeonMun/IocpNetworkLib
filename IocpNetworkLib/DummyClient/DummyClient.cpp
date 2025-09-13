// DummyClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <csmnet/network/Client.h>
#include <csmnet/network/ClientSession.h>
#include <csmnet/dto/ClientConfig.h>
#include <csmnet/util/SpdLogger.h>

#include <iostream>
#include <print>
#include <optional>

#pragma comment(lib, "IocpNetworkLib.lib")

using namespace std;
using namespace csmnet::network;
using namespace csmnet::dto;

class ClientSession2 : public ClientSession
{
public:
    using ClientSession::ClientSession;

    void OnConnected() override
    {
        _logger.Info(format("ClientSession2::OnConnected - Connected to {}:{}", _remote.GetIp(), _remote.GetPort()));

        auto sendBuffer = GetSendBuffer();
        std::span<const char> message = "This is test message.";
        if (sendBuffer.EnsureWritable(message.size()) == false)
        {
            _logger.Error("ClientSession2::OnConnected - 송신 버퍼의 크기가 부족합니다.");
            Disconnect();
            return;
        }
        auto writableSpan = sendBuffer.Poke(message.size());
        std::ranges::copy(message, reinterpret_cast<char*>(writableSpan.data()));
        sendBuffer.Advance(message.size());

        if (auto result = Send(sendBuffer); !result)
        {
            _logger.Error(format("ClientSession2::OnConnected - fail to send message : [{}] {}",
                result.error().value(),
                result.error().message()));
            Disconnect();
            return;
        }

        _logger.Info(format("ClientSession2::OnConnected - Sent : {}", message.data()));   
    }

    void OnDisconnected() override
    {
        _logger.Info(format("ClientSession2::OnDisconnected - Disconnected from {}:{}", _remote.GetIp(), _remote.GetPort()));
    }

    void OnRecv(span<const byte> message) override
    {
        _logger.Info(format("ClientSession2::OnRecv - Received : {}",
            reinterpret_cast<const char*>(message.data())));
        this_thread::sleep_for(500ms);

        auto sendBuffer = GetSendBuffer();
        if (sendBuffer.EnsureWritable(message.size()) == false)
        {
            _logger.Error("ClientSession2::OnConnected - 송신 버퍼의 크기가 부족합니다.");
            Disconnect();
            return;
        }
        auto writableSpan = sendBuffer.Poke(message.size());
        std::ranges::copy(message, writableSpan.data());
        sendBuffer.Advance(message.size());

        if (auto result = Send(sendBuffer); !result)
        {
            _logger.Error(format("ClientSession2::OnRecv - fail to send message : [{}] {}",
                result.error().value(),
                result.error().message()));
            Disconnect();
            return;
        }
        
        _logger.Info("ClientSession2::OnRecv - Echoed");
    }
};

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    auto logger = csmnet::util::SpdConsoleLogger();

    auto clientSessionFactory = [&logger]()
    {
        auto session = make_unique<ClientSession2>(logger);
        session->SetRecvBufferSize(0x10000); // 64KB
        session->SetSendBufferSize(0x10000); // 64KB
        return session;
    };

    ClientConfig config;
    config.IoThreadCount = 4;
    config.SessionCount = 1000;
    config.ServerPort = 12345;
    config.ServerIp = "127.0.0.1";
    
    Client client(clientSessionFactory, std::move(config));
    if (auto result = client.Run(); !result)
    {
        std::println("[Error: {}] Failed to connect to server: {}", static_cast<int>(result.error().value()), result.error().message());
        return 1;
    }

    while (true)
    {
        std::println("메시지를 입력하세요(Quit은 종료): ");
        std::string input;
        std::cin >> input;

        if (input == "Quit")
        {
            client.Stop();
            break;
        }
    }
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
