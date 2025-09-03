// DummyClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <csmnet/Client.h>
#include <csmnet/util/SpdLogger.h>
#include <csmnet/ClientSession.h>

#include <iostream>
#include <print>
#include <optional>

#pragma comment(lib, "IocpNetworkLib.lib")

using namespace std;
using namespace csmnet;

class ClientSession2 : public ClientSession
{
public:
    using ClientSession::ClientSession;

    void OnConnected() override
    {
        ClientSession::OnConnected();

        _logger.Info(format("ClientSession2::OnConnected - Connected to {}:{}", _remote.GetIp(), _remote.GetPort()));
    }

    void OnDisconnected() override
    {
        ClientSession::OnDisconnected();
        
        _logger.Info(format("ClientSession2::OnDisconnected - Disconnected from {}:{}", _remote.GetIp(), _remote.GetPort()));
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
        return session;
    };

    ClientConfig config;
    config.IoThreadCount = 4;
    config.SessionCount = 100;
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
        std::println("Press any key to exit...");
        std::string input;
        std::cin >> input;
        if (input == "q" || input == "Q")
        {
            client.Stop();
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
