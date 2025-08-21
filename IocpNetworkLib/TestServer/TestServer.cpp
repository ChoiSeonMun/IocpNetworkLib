// TestServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <csmnet/Server.h>

#include <iostream>
#include <print>

#pragma comment(lib, "IocpNetworkLib.lib")


int main()
{
    SetConsoleOutputCP(CP_UTF8);

    csmnet::Server server;

    if (auto result = server.Open(1000, 12345); !result)
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
