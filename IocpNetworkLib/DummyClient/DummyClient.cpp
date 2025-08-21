// DummyClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <csmnet/Client.h>
#include <csmnet/util/SpdLogger.h>

#include <iostream>
#include <print>
#include <optional>

#pragma comment(lib, "IocpNetworkLib.lib")

using namespace std;
using namespace csmnet;

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    auto server = Endpoint::From("127.0.0.1", 12345);
    if (!server)
    {
        std::println("[Error] Invalid server endpoint.");
        return 1;
    }

    Client client;
    if (auto result = client.Connect(server.value()); !result)
    {
        std::println("[Error: {}] Failed to connect to server: {}", static_cast<int>(result.error().value()), result.error().message());
        return 1;
    }

    std::println("Connected to server at {}:{}", server->GetIp(), server->GetPort());
    while (true)
    {
        std::println("Press any key to exit...");
        char input;
        std::cin >> input;
        if (input == 'q' || input == 'Q')
        {
            client.Disconnect();
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
