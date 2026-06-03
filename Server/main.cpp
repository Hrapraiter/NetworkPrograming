//Server
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <FormatLastError.h>

using namespace std;

#pragma comment(lib , "WS2_32.lib") 

#define MTU 1500

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
		cout << "SOCKET creation failed with " << FormatLastError((dwError = WSAGetLastError()) , szError) << endl; 
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// 4) Привязываем сокет к интерфейсу и порту:
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);

	if (iResult != 0)
	{
		cout << "bind failed with " << FormatLastError((dwError = WSAGetLastError()) , szError) << endl;
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
	

	SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed with " << FormatLastError((dwError = WSAGetLastError()) , szError) << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	sockaddr_in client_sockaddr;// буфер для хранения клиента
	int client_addrLen = sizeof(client_sockaddr); // размер буфера
	iResult = getpeername(client_socket, (sockaddr*)&client_sockaddr, &client_addrLen); // заполнение буфера клиента
	CHAR ip_buffer[INET_ADDRSTRLEN] = {}; // INET_ADDRSTRLEN = 22 хотя в теории должен быть 16 
	DWORD port = 0; 
	if(iResult == 0)
	{
		inet_ntop(AF_INET, &client_sockaddr.sin_addr, ip_buffer, INET_ADDRSTRLEN);	// приводит бинарное представление адреса к строковому формату
																					// заполняя буфер под строку  
		port = ntohs(client_sockaddr.sin_port); // ntohs нужен для чтения 16 битных значений 
	}
	cout << "Connect : " << ip_buffer << ':' << port << "\n\n";
	//7) Получаем данные от клиента:
	
	CHAR recv_buffer[MTU] = {};
	CHAR send_buffer[MTU] = "Hello Client";
	INT iReceivedBytes = 0;
	INT iSendBytes = 0;

	do
	{
		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		dwError = WSAGetLastError();
		if (iReceivedBytes > 0)
		{
			cout << "Received " << iReceivedBytes << " " << recv_buffer << endl;
			iSendBytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			if (iSendBytes == SOCKET_ERROR)
				cout << "Send failed with " << FormatLastError(dwError , szError) << endl;
			else cout << iSendBytes << " Bytes sent" << endl;
		}
		else if (iReceivedBytes == 0)cout << "Connection closing..." << endl;
		else cout << "Receive failed with " << FormatLastError(dwError , szError) << endl;

	} while (iReceivedBytes > 0);

	//8) Разрываем TCP-соединение 
	iResult = shutdown(client_socket, SD_BOTH);
	if(iResult != SOCKET_ERROR)
		cout << "shutdown failed with error:\t" << FormatLastError((dwError = WSAGetLastError()) , szError) << endl;
	
	//?) Освобождаем ресурсы занятые WinSOCK:
	
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}