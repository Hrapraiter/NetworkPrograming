//Client
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
// если с бибилиотекой WinSOCK подключается файл <Windows.h>
// или <IPhlpAPI.h> то они тоже подключают файл <WinSOCK2.h> что
// приводит к конфликтам .
// Для того чтобы <Windows.h> и <IPhlpAPI.h> не подтягивали
// WinSOCK, создаётся макроопределение.
#endif // !WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <FormatLastError.h>

using namespace std;

#pragma comment(lib , "WS2_32.lib") // втраиваем статическую
// библиотеку, для заголовка <WS2TCPIP.h>

#define MTU 1500 // Maximum Transfer Unit - Максимально-возможный размер Ethernet-кадра

void main()
{
	setlocale(LC_ALL, "");
	cout << "Client\n\n";

	DWORD dwError = 0;
	CHAR szError[256] = {};

	//1) Инициализация WinSOCK
	WSAData wsaData;
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(iResult != 0)
	{
		cout << "WinSOCK init failed with code: " << iResult;
		return;
	}

	//2) Определяем параметры подключения:
	addrinfo hints;
	addrinfo* target;
	ZeroMemory(&hints, sizeof(hints));	// Обнуляем экземпляр структуры 
	hints.ai_family = AF_INET;			//Стек протколов TCP/IPv4 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;	// Определяем протокол транспортного уровня 

	CHAR ip[INET_ADDRSTRLEN] = {};
	CHAR port[6] = {};

	cout << "ip : "; cin >> ip;
	cout << "port : "; cin >> port;

	//iResult = getaddrinfo("127.0.0.1","27015",&hints,&target);
	iResult = getaddrinfo(ip, port , &hints, &target);
	if(iResult != 0)
	{
		cout << "getaddressinfo() failed with code " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	//3) Создаём сокет:
	SOCKET connect_socket = socket (target->ai_family ,target->ai_socktype ,target->ai_protocol);
	dwError = WSAGetLastError();
	if(connect_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error:\t" << dwError << endl;
		cout << FormatLastError(dwError, szError) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	
	//4) Подключаемся к узлу:
	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	dwError = WSAGetLastError();
	freeaddrinfo(target);
	if(iResult == SOCKET_ERROR)
	{
		//cout << "Error: " << dwError << ":\t";
		
		cout << FormatLastError(dwError , szError) << endl;
		//cout << lpError << '\n';
		// WSAGetLastError в обязательном порядке должна быть
		// после вызова функции которая потенциально может
		// выполнится с ошибкой.
		cout << "Unable to connect to server" << endl;
		closesocket(connect_socket);
		WSACleanup();
		return;
	}

	//5) Отправка:
	CHAR send_buffer[MTU] = "Hello Server";
	iResult = send(connect_socket,send_buffer , strlen(send_buffer),0);
	dwError = WSAGetLastError();
	if(iResult == SOCKET_ERROR)
	{
		cout << "Send failed with error: " << dwError << endl;
		cout << FormatLastError(dwError, szError);
		closesocket(connect_socket);
		WSACleanup();
		return;
	}
	//6) Получение данных:
	CHAR recv_buffer[MTU] = {};
	do
	{
		iResult = recv(connect_socket, recv_buffer, MTU, 0);
		dwError = WSAGetLastError();
		if (iResult > 0)		cout << "Bytes received: " << iResult << " Message: " << recv_buffer << endl;
		else if (iResult == 0)	cout << "Connection closed" << endl;
		else					cout << "Receive failed with error " << FormatLastError(dwError , szError) << endl;
		
	} while (iResult > 0);

	iResult = shutdown(connect_socket, SD_BOTH); // Закрываем сокет на получение и отправку данных (разрываем TCP-соединение) :
	if (iResult == SOCKET_ERROR)
		cout << "Shutdown failed with error " << FormatLastError(WSAGetLastError() , szError) << endl;

	//7) Освобождаем ресурсы WinSOCK
	closesocket(connect_socket);
	WSACleanup();
}