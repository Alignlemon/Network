#include <winsock2.h>

#include <iostream>
#include <thread>
#include <string>
#include <conio.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define TEST_IP "127.0.0.1"
#define TEST_PORT 1234

void ErrorHandling(const char* message);
void receiveMessages(SOCKET sock);
void clearLine();

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN servAddr;

    char ch;
    std::string name;
    std::string message;

    //if (argc != 3)
    //{
    //    printf("Usage : %s <IP> <port>\n", argv[0]);
    //    exit(1);
    //}

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
        ErrorHandling("hSocket() error");

    ZeroMemory(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(TEST_IP);
    servAddr.sin_port = htons(TEST_PORT);

    if (connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) < 0)
    {
        std::cerr << "서버에 연결할 수 없습니다!" << std::endl;
        return 1;
    }

    std::thread(receiveMessages, clientSocket).detach();

    std::getline(std::cin, name);
    clearLine();
    send(clientSocket, name.c_str(), name.size(), 0);

    while (true)
    {
        std::getline(std::cin, message);
        clearLine();
        send(clientSocket, message.c_str(), message.size(), 0);
        message = "";
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

void ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void receiveMessages(SOCKET sock)
{
    constexpr const int bufferSize = 1024;
    char buffer[bufferSize];

    while (true)
    {
        int recvSize = recv(sock, buffer, bufferSize - 1, 0);

        if (recvSize <= 0)
        {
            std::cout << "서버와의 연결이 종료되었습니다.\n" << std::endl;
            break;
        }
        buffer[recvSize] = '\0';
        std::cout << buffer << std::endl;
    }
}

void clearLine()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    COORD cursorPosition = csbi.dwCursorPosition;

    cursorPosition.Y = (cursorPosition.Y > 0) ? cursorPosition.Y - 1 : 0;
    cursorPosition.X = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);

    for (int i = 0; i < csbi.dwSize.X; ++i)
    {
        std::cout << ' ';
    }

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}
