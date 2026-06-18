#include <iostream>
#include <chrono>
#include <type_traits>
#include <Windows.h>
#include <conio.h>
using std::cin;
using std::cout;
using std::endl;

#define DEBUG
#define THREAD_SECURITY_FOR_UNREUSING(hThread , work_status)\
{\
	if(hThread != NULL)\
	{\
		WaitForSingleObject(hThread, INFINITE); \
		CloseHandle(hThread); \
		hThread = NULL;\
	}\
}
#define Escape	 27
#define Enter	 13
#define I		'i'

#define MIN_TANK_CAPACITY	 20
#define MAX_TANK_CAPACITY	120

class Tank
{
	const int CAPACITY;
	double fuel_level;
public:
	Tank(int capacity):
		CAPACITY
		(
			capacity < MIN_TANK_CAPACITY ? MIN_TANK_CAPACITY :
			capacity > MAX_TANK_CAPACITY ? MAX_TANK_CAPACITY :
			capacity
		)
	{
		this->fuel_level = 0;
		cout << "Tank is ready: \t\t" << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over " << this << endl;
	}
	double get_fuel_level()const { return fuel_level; }
	void fill(int amount)
	{
		if (amount < 0)return;
		fuel_level += amount;
		if (fuel_level > CAPACITY)fuel_level = CAPACITY;
	}
	double give_fuel(double amount)
	{
		if (amount < 0)return fuel_level;
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}
	void info()const
	{
		cout << "Capacity:\t" << CAPACITY << " liters.\n";
		cout << "Fuel level:\t" << fuel_level << " liters.\n";
	}
};

#define MIN_ENGINE_CONSUMPTION	 4
#define MAX_ENGINE_CONSUMPTION	30


class Engine
{
	HANDLE hThread = NULL;
	const double CONSUMPTION;		// Расход на 100км
	double consumption_per_second;	// Расход за 1 секунду
	bool isWork = false;
public:
	Engine(double consumption):CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5; // 3 * 10^(-5)
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		isWork = false;
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		cout << "Engine is over:\t\t" << this << endl;
	}
	void start(){ isWork = true; }
	void stop() { isWork = false; }
	void info()const
	{
		cout << "Consumption:\t\t" << CONSUMPTION << " liters/km.\n";
		cout << "Consumption per sec:\t" << consumption_per_second << " liters/sec.\n";
	}
	void thread_engine_work(Tank* tank)
	{
		/*if(hThread != NULL)
		{
			bool is_work_tmp = isWork;
			if (isWork)isWork = false;

			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = NULL;
			
			isWork = is_work_tmp;
		}*/
		THREAD_SECURITY_FOR_UNREUSING(hThread, isWork);
		std::pair<Engine*, Tank*>* args = new std::pair<Engine* , Tank*>{this , tank};
		hThread = CreateThread
		(
			NULL,
			NULL,
			(LPTHREAD_START_ROUTINE)Thread_Engine_Transport,
			(LPVOID)args,
			NULL,
			NULL
		);
		if(hThread == NULL)
		{
			delete args;
			args = nullptr;
		}
	}
private:
	static void Thread_Engine_Transport(std::pair<Engine* , Tank*>* args)
	{
		cout << "Thread_Engine start" << endl;
		args->first->thread_func(args->second);
		delete args;
		args = nullptr;
		cout << "Thread_Engine end" << endl;
	}
	void thread_func(Tank* tank)
	{
		while(isWork)
		{
			Sleep(1000);
			tank->give_fuel(consumption_per_second);
		}
	}
};
class Car
{
	HANDLE hThread_panel = NULL;
	Engine engine;
	Tank tank;
	bool driver_inside;
	bool is_enable_I;
public:
	Car(double consumption, int capacity = 50) :engine(consumption), tank(capacity)
	{
		driver_inside = false;
		is_enable_I = false;
		cout << "Your car is ready to go, press Enter to get in " << this << endl;
	}
	~Car()
	{
		driver_inside = false;
		is_enable_I = false;
		WaitForSingleObject(hThread_panel, INFINITE);
		CloseHandle(hThread_panel);
		cout << "Car is over: " << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		thread_panel();
	}
	void get_out()
	{
		driver_inside = false;
	}
	inline void fill(double amount) { tank.fill(amount); }
	void control()
	{
		char key = 0;
		do
		{
			key = _getch(); // Функция _getch() ожидает  нажатие клавиши и возвоащает её ASCII-код
			switch(key)
			{
			case Enter:
				if (driver_inside)get_out();
				else get_in();
				break;
			case I:
				if(is_enable_I)
				{
					engine.stop();
					is_enable_I = false;
				}
				else
				{
					engine.start();
					engine.thread_engine_work(&tank);
					is_enable_I = true;
				}
				break;
			}
		} while (key != Escape);
	}
	void show_all_info()
	{
		tank.info();
		engine.info();
	}
	bool get_is_enable_I()const { return is_enable_I; }
	void thread_panel()
	{
		THREAD_SECURITY_FOR_UNREUSING(hThread_panel, driver_inside);
		hThread_panel = CreateThread
		(
			NULL,
			NULL,
			(LPTHREAD_START_ROUTINE)Thread_Transport,
			(LPVOID)this,
			NULL,
			NULL
		);
	}
	void panel()
	{
		while (driver_inside)
		{
			system("CLS");
			//cout << "Fuel level: " << tank.get_fuel_level() << " liters.\n" << endl;
			show_all_info();
			Sleep(100);
		}
	}
private:
	static void Thread_Transport(Car* car)
	{
		car->panel();
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main() 
{
	setlocale(LC_ALL, "");
#ifdef TANK_CHECK
	Tank tank(40);
	int amount;
	while (true)
	{
		cout << "Введите объём топлива: "; cin >> amount;
		tank.fill(amount);
		tank.info();
	}
#endif // TANK_CHECK
#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif //  ENGINE_CHECK

	Car bmw(10, 70);
	bmw.fill(40);
	bmw.control();
}