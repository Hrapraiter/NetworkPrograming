#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <thread>
#include <chrono>
using std::cin;
using std::cout;
using std::endl;
using namespace std::chrono_literals;

#define Escape	27
#define Enter	13

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
		cout << "Tank is over: \t\t" << this << endl;
	}
	double get_fuel_level()const { return fuel_level; }
	void fill(double amount)
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
	const double CONSUMPTION;		// Расход на 100км
	double consumption_per_second;	// Расход за 1 секунду
	bool is_started;
public:
	Engine(double consumption):CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5; // 3 * 10^(-5)
		is_started = false;
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t\t" << this << endl;
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started() const { return is_started; }
	double get_consumption_per_second() const { return consumption_per_second; }
	void info()const
	{
		cout << "Consumption:\t\t" << CONSUMPTION << " liters/km.\n";
		cout << "Consumption per sec:\t" << consumption_per_second << " liters/sec.\n";
	}
};
class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	struct
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
	}car_threads;
public:
	Car(double consumption, int capacity = 50) :engine(consumption), tank(capacity)
	{
		driver_inside = false;
		cout << "Your car is ready to go, press Enter to get in \t" << this << endl;
	}
	~Car()
	{
		cout << "Car is over: \t\t" << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		//panel();
		if (!car_threads.panel_thread.joinable())
			car_threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (car_threads.panel_thread.joinable())
			car_threads.panel_thread.join();
		system("CLS");
		cout << "You are out of the car" << endl;
	}
	void startup()
	{
		if(tank.give_fuel(0))
		{
			engine.start();
			if(!car_threads.engine_idle_thread.joinable())
				car_threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}
	void shutdown()
	{
		engine.stop();
		if (car_threads.engine_idle_thread.joinable())
			car_threads.engine_idle_thread.join();
	}
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
			case 'F':
			case 'f':
				if (!driver_inside || !engine.started())
				{
					double amount;
					cout << "Введите объём топлива: "; cin >> amount;
					tank.fill(amount);
				}
				else cout << "нужно заглушить двигатель и выйти из машины, у нас только самообслуживание." << endl;
				break;
			case 'I':
			case 'i':
				if (!engine.started())startup();
				else shutdown();
				break;
			case Escape:
				shutdown();
				get_out();
				break;
			}
		} while (key != Escape);
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second()))
			std::this_thread::sleep_for(1s);
	}
	void panel()
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		while (driver_inside)
		{
			system("CLS");
			cout << "Fuel level: " << tank.get_fuel_level() << " liters.\t";
			if (tank.get_fuel_level() < 5)
			{
				SetConsoleTextAttribute(hConsole, 0x4F);
				cout << " LOW FUEL ";
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			cout << endl;
			cout << "Engine is " << (engine.started() ? "started" : "stoped") << endl;
			std::this_thread::sleep_for(100ms);
		}
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
	bmw.control();

}