#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <mutex>
using std::cout;
using std::cin;
using std::endl;
using namespace std::chrono_literals;

bool finish = false;
std::mutex mtx;
HANDLE ghMutex = NULL;


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
		//mtx.lock();
		WaitForSingleObject(ghMutex, INFINITE);
		cout << "+ ";
		Sleep(100);
		ReleaseMutex(ghMutex);
		//mtx.unlock();
		//std::this_thread::sleep_for(100ms);
	}
}
VOID Minus()
{
	while (!finish)
	{
		//mtx.lock();
		WaitForSingleObject(ghMutex, INFINITE);
		cout << "- ";
		Sleep(100);
		ReleaseMutex(ghMutex);
		//mtx.unlock();
		//std::this_thread::sleep_for(100ms);
	}
}
//#define WINDOWS_THREADS_1
//#define WINDOWS_THREADS_2
//#define CPP_THREADS

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
#ifdef CPP_THREADS
	cout << "Start" << endl;
	std::thread thread_plus = std::thread(Plus);
	std::thread thread_minus = std::thread(Minus);
	cin.get();
	finish = true;
	cout << "End" << endl;
	if (thread_plus.joinable())thread_plus.join();
	if (thread_minus.joinable())thread_minus.join();
#endif // CPP_THREADS

	ghMutex = CreateMutex(NULL, FALSE, NULL);
	
	HANDLE hThreads[2] = {};
	hThreads[0] = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Plus,
		NULL,
		NULL,
		0
	);
	hThreads[1] = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Minus,
		NULL,
		NULL,
		0
	);
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

}

