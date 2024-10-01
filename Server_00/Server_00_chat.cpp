#include <winsock2.h>

#include <iostream>
#include <thread>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

struct CLIENT
{
	SOCKET m_ClientSock;
	std::string m_ClientName;
};

void ErrorHandling(const char* message);
void handleClient(int clientIndex, std::vector<CLIENT>& clients);
void sendMessageToAll(std::vector<CLIENT>& clients, std::string message);
void sendMessageToAll(int clientIndex, std::vector<CLIENT>& clients, std::string message);

#define TEST_PORT 1234

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock; // , hClntSock;
	std::vector<CLIENT> Clients;
	SOCKADDR_IN servAddr, clntAddr;

	int szClntAddr = sizeof(clntAddr);
	char message[] = "Hello World!";

	//if (argc != 2)
	//{
	//	printf("Usage : %s <port>\n", argv[0]);
	//	exit(1);
	//}

	// WSAStartup(윈속 버전, )
	// MAKEWORD(주버전, 부버전)
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(TEST_PORT);
	//servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	std::cout << "Server start!\n";

	while (1)
	{
		CLIENT ClientTemp;
		ClientTemp.m_ClientSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (ClientTemp.m_ClientSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
			continue;
		}

		Clients.push_back(ClientTemp);

		int clientIndex = Clients.size() - 1;
		std::thread(handleClient, clientIndex, std::ref(Clients)).detach();
	}

	for (auto& data : Clients)
	{
		closesocket(data.m_ClientSock);
	}

	closesocket(hServSock);
	WSACleanup();

	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void handleClient(int clientIndex, std::vector<CLIENT>& clients)
{
	constexpr const int bufferSize = 1024;
	char buffer[bufferSize];

	send(clients[clientIndex].m_ClientSock, "이름을 입력하세요: ", 20, 0);
	int recvSize = recv(clients[clientIndex].m_ClientSock, buffer, bufferSize - 1, 0);
	if (recvSize <= 0)
		return;

	buffer[recvSize] = '\0';
	clients[clientIndex].m_ClientName = std::string(buffer);
	std::cout << clients[clientIndex].m_ClientName << "님이 접속했습니다.\n";

	std::string welcomeMessage = clients[clientIndex].m_ClientName + "님이 채팅에 참여했습니다.";
	sendMessageToAll(clients, welcomeMessage);

	while (true)
	{
		recvSize = recv(clients[clientIndex].m_ClientSock, buffer, bufferSize - 1, 0);
		if (recvSize <= 0)
		{
			std::cout << clients[clientIndex].m_ClientName + "님이 연결을 종료했습니다.\n";
			std::string exitMessage = clients[clientIndex].m_ClientName + "님이 연결을 종료했습니다.";
			sendMessageToAll(clients, exitMessage);
			break;
		}
		buffer[recvSize] = '\0';

		std::string message = clients[clientIndex].m_ClientName + ": " + std::string(buffer);
		sendMessageToAll(clients, message);
	}

	closesocket(clients[clientIndex].m_ClientSock);
}

void sendMessageToAll(std::vector<CLIENT>& clients, std::string message)
{
	for (auto& data : clients)
	{
		send(data.m_ClientSock, message.c_str(), message.size(), 0);
	}
}

void sendMessageToAll(int clientIndex, std::vector<CLIENT>& clients, std::string message)
{
	for (int i = 0; i < clients.size(); ++i)
	{
		if (i != clientIndex)
			send(clients[i].m_ClientSock, message.c_str(), message.size(), 0);
	}
}
