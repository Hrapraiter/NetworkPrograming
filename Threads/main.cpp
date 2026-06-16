#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
using std::cout;
using std::cin;
using std::endl;
using namespace std::chrono_literals;

bool finish = false;
VOID Function()
{
	while (!finish)
	{
		cout << "Hello Threads " << GetCurrentThreadId() << endl;
		//system("PAUSE");
	}
}
struct Point
{
	int x = 0;
	int y = 0;
};
VOID Collision(Point* point)
{
	while (point->x != point->y)
	{
		cout << "X = " << point->x++ << "\tY = " << point->y-- << '\n';
		Sleep(10);
	}
}
VOID Decrement(int i)
{
	while (i)cout << i-- << "\t";
}
VOID Plus()
{
	while(!finish)
	{
		cout << "+ ";
		std::this_thread::sleep_for(100ms);
	}
}
VOID Minus()
{
	while (!finish)
	{
		cout << "- ";
		std::this_thread::sleep_for(100ms);
	}
}
//#define WINDOWS_THREADS_1
//#define WINDOWS_THREADS_2

void main()
{
	setlocale(LC_ALL, "");
#ifdef WINDOWS_THREADS_1
	DWORD dwID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Function,
		NULL,
		NULL,
		&dwID
	);
	cin.get();
	finish = true;
	cout << "Thread ID from main(): " << dwID << endl;
	WaitForSingleObject(hThread, INFINITE);
#endif // WINDOWS_THREADS_1
#ifdef WINDOWS_THREADS_2
	Point A{ 0 , 50 };
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Collision,
		(LPVOID)&A, // LPVOID - LongPointer может хранить указатель на абсолютно любой тип данных
		NULL,
		&dwThreadID
	);
	WaitForSingleObject(hThread, INFINITE);

#endif // WINDOWS_THREADS_2
	/*std::thread threads[2] = {};
	cout << "Start" << endl;
	threads[0] = std::thread(Plus);
	threads[1] = std::thread(Minus);
	cin.get();
	finish = true;
	cout << "End" << endl;
	for (int i = 0; i < 2; i++)
		if (threads[i].joinable())
			threads[i].join();*/

	while(true)
	{
		cout << std::this_thread::get_id() << "\t";
		//std::this_thread::sleep_for(100ms);
	}
}

