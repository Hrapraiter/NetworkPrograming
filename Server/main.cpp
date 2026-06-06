//Server
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <thread>
#include <list>
#include <FormatLastError.h>

using namespace std;

#pragma comment(lib , "WS2_32.lib") 
#define MTU 1500

DWORD dwError = 0;
CHAR szError[256] = {};



struct Client
{
	//client 
	SOCKET client_socket = INVALID_SOCKET;
	CHAR ip_buffer[INET_ADDRSTRLEN] = {};
	DWORD port = 0;
	CHAR recv_buffer[MTU] = {};
	//server for client
	INT iReceivedBytes = 0;
	INT iSendBytes = 0;

	thread session;
	// struct logic
	Client(SOCKET _client_socket)
	{
		client_socket = _client_socket;
		UpdateIPAndPort();
		cout << "Connect " << ip_buffer << ':' << port << endl;
		session = thread([this] {logic(); });
	}
	/*Client(Client&& client)noexcept
	{

		client_socket = exchange(client.client_socket, INVALID_SOCKET);
		session = move(client.session);

		strcpy(ip_buffer, client.ip_buffer);
		ZeroMemory(client.ip_buffer, INET_ADDRSTRLEN);

		port = exchange(client.port, 0);

		strcpy(recv_buffer, client.recv_buffer);
		ZeroMemory(client.recv_buffer, MTU);

		iReceivedBytes = exchange(client.iReceivedBytes, 0);
		iSendBytes = exchange(client.iSendBytes, 0);
	}
	Client& operator=(Client&& client)noexcept
	{
		if (this != &client)
		{
			if (session.joinable())session.join();

			client_socket = exchange(client.client_socket, INVALID_SOCKET);
			session = move(client.session);

			strcpy(ip_buffer, client.ip_buffer);
			ZeroMemory(client.ip_buffer, INET_ADDRSTRLEN);

			port = exchange(client.port, 0);

			strcpy(recv_buffer, client.recv_buffer);
			ZeroMemory(client.recv_buffer, MTU);

			iReceivedBytes = exchange(client.iReceivedBytes, 0);
			iSendBytes = exchange(client.iSendBytes, 0);
		}
		return *this;
	}*/
	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;
	~Client()
	{
		INT iResult = shutdown(client_socket, SD_BOTH);
		if (iResult != SOCKET_ERROR)
			cout << "shutdown failed with error:\t" << WSAGetLastError() << endl;
		closesocket(client_socket);

		if (session.joinable())session.join();

	}

	void UpdateIPAndPort()
	{
		static SOCKADDR_IN client_address;
		static INT client_address_len = sizeof(client_address);

		getpeername(client_socket, (SOCKADDR*)&client_address, &client_address_len);
		inet_ntop(AF_INET, &client_address.sin_addr, ip_buffer, INET_ADDRSTRLEN);
		port = ntohs(client_address.sin_port);
	}
private:
	void logic()
	{
		do
		{
			ZeroMemory(recv_buffer, MTU);
			iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
			if (iReceivedBytes > 0)
			{
				cout << "Received " << iReceivedBytes << "\n " << ip_buffer << ':' << port << " -> " << recv_buffer << endl;
				iSendBytes = send(client_socket, recv_buffer, strlen(recv_buffer), 0);
				if (iSendBytes == SOCKET_ERROR)
					cout << "Send failed with error:\t" << WSAGetLastError() << endl;
				else cout << iSendBytes << " Bytes sent" << endl;
			}
			else if (iReceivedBytes == 0)cout << "Connection closing..." << endl;
			else cout << "Receive failed with error: " << FormatLastError(WSAGetLastError(), szError) << endl;
		} while (iReceivedBytes > 0);
	}
};

list<Client> clients;
bool isWork = true;
void client_listener(SOCKET listen_socket)
{
	while (isWork)
	{
		SOCKET client_socket = accept(listen_socket, NULL, NULL);
		if (!isWork)break;
		if (client_socket == INVALID_SOCKET)
		{
			cout << "Accept failed with error: " << WSAGetLastError() << endl;
			closesocket(client_socket);
			continue;
		}
		clients.emplace_back(client_socket); // emplace_back это создание в узле без копирования как в (push_back)
	}
}

void main()
{
	setlocale(LC_ALL, "");
	//1) Инициализация WinSOCK:
	cout << "Server\n\n";

	WSADATA wsaDATA;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaDATA);

	if (iResult != 0)
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		WSACleanup();
		return;
	}

	//2) Параметры подключения

	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	// Соединение будет работать в режиме LISTENING


	iResult = getaddrinfo(NULL, "27015", &hints, &target);


	if (iResult != 0)
	{
		cout << "getaddinfo() failed with error: " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	// 3) Создание серверного сокета, который будет постоянно прослушивать: 
	SOCKET listen_socket =
		socket(target->ai_family, target->ai_socktype, target->ai_protocol);

	if (listen_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// 4) Привязываем сокет к интерфейсу и порту:
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);

	if (iResult != 0)
	{
		cout << "bind failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//5) Запускаем прослушивание порта:
	if (listen(listen_socket, 1) == SOCKET_ERROR)// 1 максимальное колличество одновременно подключенных клиентов
	{
		cout << "Listen failed with error: " << WSACleanup() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//6) Принимаем подключение от клиента
	/*
	SOCKADDR_IN client_address;
	INT client_address_len = sizeof(client_address);
	SOCKET client_socket = accept(listen_socket, (SOCKADDR*)&client_address, &client_address_len);
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	cout << inet_ntoa(client_address.sin_addr) << ':' << ntohs(client_address.sin_port) << '\n';
	*/
	//7) Получаем данные от клиента:
	CHAR send_buffer[MTU] = "Hello Client";

	thread listener(&client_listener, listen_socket);

	cout << "Prees to stop server."; cin.get();

	isWork = false;
	listener.join();
	clients.clear();

	//8) Разрываем TCP-соединение 

	//?) Освобождаем ресурсы занятые WinSOCK:
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}