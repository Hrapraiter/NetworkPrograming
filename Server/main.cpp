//Server
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <FormatLastError.h>
#include <Messages.h>
using namespace std;

#pragma comment(lib , "WS2_32.lib") 

#define MTU					 1500
#define MAX_CONNECTIONS			3

SOCKET	client_sockets[MAX_CONNECTIONS] = {};
DWORD	dwThreadIDs[MAX_CONNECTIONS] = {};		// Идентификаторы потоков
HANDLE	hThreads[MAX_CONNECTIONS] = {};			// Дескрипторы потоков

INT g_ActiveClients = 0;

VOID ClientHandle(SOCKET client_socket);
VOID ShowActiveClients();

void main()
{
	setlocale(LC_ALL, "");
	//1) Инициализация WinSOCK:
	DWORD dwError = 0;
	CHAR szError[256] = {};
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
	if (listen(listen_socket, MAX_CONNECTIONS) == SOCKET_ERROR)// 1 максимальное колличество одновременно подключенных клиентов
	{
		cout << "Listen failed with error: " << WSACleanup() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	do
	{
		//6) Принимаем подключение от клиента
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
		//7) Получаем данные от клиента:
		//ClientHandle(client_socket);
		if (g_ActiveClients < MAX_CONNECTIONS)
		{
			client_sockets[g_ActiveClients] = client_socket;
			hThreads[g_ActiveClients] = CreateThread
			(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ClientHandle,
				(LPVOID)client_sockets[g_ActiveClients],
				NULL,
				&dwThreadIDs[g_ActiveClients]
			);
			g_ActiveClients++;
			ShowActiveClients();
		}
		else
		{
			cout << "DECLINED" << endl;
			iResult = send(client_socket ,DECLINE_MESSAGE,strlen(DECLINE_MESSAGE) , 0);
			dwError = WSAGetLastError();
			if(iResult != 0)cout << FormatLastError(dwError, szError) << '\n';
			iResult = shutdown(client_socket, SD_BOTH);
			dwError = WSAGetLastError();
			if(iResult != 0)cout << FormatLastError(dwError , szError) <<'\n';

			closesocket(client_socket);				   
			dwError = WSAGetLastError();
			if (iResult != 0)cout << FormatLastError(dwError, szError) << '\n';
			
		}
	} while (true);

	WaitForMultipleObjects(g_ActiveClients, hThreads, TRUE, INFINITE);
	
	//?) Освобождаем ресурсы занятые WinSOCK:
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}
INT GetThreadIndex(DWORD dwThreadID)
{
	for(INT i = 0;i < g_ActiveClients;i++)
		if (dwThreadID == dwThreadIDs[i])return i;
	return -1;
}
VOID Shift(INT index)
{
	if (index == -1)return;
	CloseHandle(hThreads[index]);
	for(INT i = index;i < g_ActiveClients;i++)
	{
		client_sockets[i] = client_sockets[i + 1];
		dwThreadIDs[i] = dwThreadIDs[i + 1];
		hThreads[i] = hThreads[i + 1];
	}
	client_sockets[MAX_CONNECTIONS - 1] = NULL;
	dwThreadIDs[MAX_CONNECTIONS - 1] = NULL;
	hThreads[MAX_CONNECTIONS - 1] = NULL;
	g_ActiveClients--;
	ShowActiveClients();
}
BOOL IsShowActiveCients = false; // 1
VOID ShowActiveClients()
{
	IsShowActiveCients = true; // 2
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	SetConsoleCursorPosition(hConsole, { 25 , 0 });
	cout << "Колличество клиентов: " << g_ActiveClients << '\n';
	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
	IsShowActiveCients = false; // 3
}
VOID ClientHandle(SOCKET client_socket) 
{
	INT iResult = 0;
	DWORD dwError = 0;
	CHAR szError[256] = {};
	
	CHAR send_buffer[MTU] = "Hello Client";
	INT iReceivedBytes = 0;
	INT iSendBytes = 0;

	CHAR recv_buffer[MTU] = {};
	while (IsShowActiveCients); // 4 решение в 4 строки просто ждём пока ShowActiveClients отработает
								//  до конца в основном потоке программы чтобы не воевать с текущим 
								//  потоком за кисть окна консоли
	do
	{
		ZeroMemory(recv_buffer, MTU);
		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		dwError = WSAGetLastError();
		if (iReceivedBytes > 0)
		{
			cout << "Received " << iReceivedBytes << " " << recv_buffer << endl;
			iSendBytes = send(client_socket, recv_buffer, strlen(recv_buffer), 0);
			if (iSendBytes == SOCKET_ERROR)
				cout << "Send failed with error:\t" << FormatLastError(dwError , szError) << endl;
			else cout << iSendBytes << " Bytes sent" << endl;
		}
		//else if (iReceivedBytes == 0)cout << "Connection closing..." << endl;
		else cout << "Receive failed with error: " << FormatLastError(dwError , szError) << endl;


	} while (iReceivedBytes > 0 && strcmp(recv_buffer, "exit") != 0);

	//8) Разрываем TCP-соединение 
	iResult = shutdown(client_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult != SOCKET_ERROR)
		cout << "shutdown failed with error:\t" << FormatLastError(dwError , szError) << endl;
	closesocket(client_socket);
	Shift(GetThreadIndex(GetCurrentThreadId()));
	ExitThread(0);
}