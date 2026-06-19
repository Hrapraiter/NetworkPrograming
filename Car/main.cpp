#include <Windows.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <thread>
#include <mutex>
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
	void consumption_to_speed(const int& speed)
	{
		static bool(*range)(int, int , int) = [](int start, int end , int value)
			{
				return value >= start && value <= end;
			};
		if (range(1, 60, speed))			consumption_per_second = 0.0020;
		else if (range(61, 100, speed))		consumption_per_second = 0.0014;
		else if (range(101 , 140, speed))	consumption_per_second = 0.0020;
		else if (range(141, 200, speed))	consumption_per_second = 0.0025;
		else if (range(200, 250, speed))	consumption_per_second = 0.0030;
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
	int speed;
	const int speed_tick = 2;
	const int max_speed = 250;
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
		speed = 0;
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
			key = 0;
			if(_kbhit())key = _getch(); // Функция _getch() ожидает  нажатие клавиши и возвоащает её ASCII-код
			switch(key)
			{
			case Enter:
				if (driver_inside)get_out();
				else get_in();
				break;
			case 'W':
			case 'w':
				if (driver_inside && engine.started())
				{
					speed += speed_tick;
					if (speed > 250)speed = 250;
					engine.consumption_to_speed(speed);
				}
				break;
			case 'S':
			case 's':
				if (driver_inside && engine.started())
				{
					speed -= speed_tick * 2;
					if (speed < 0)speed = 0;
					engine.consumption_to_speed(speed);
				}
				break;
			case 'F':
			case 'f':
			{
				CONSOLE_SCREEN_BUFFER_INFO info;
				if (!driver_inside || !engine.started())
				{
					double amount;
					if (driver_inside)
					{
						mut_cout.lock();

						GetConsoleScreenBufferInfo(hConsole, &info);
						SetConsoleCursorPosition(hConsole, cords.end);
						cout << "Введите объём топлива: "; cin >> amount;
						tank.fill(amount);
						SetConsoleCursorPosition(hConsole, info.dwCursorPosition);

						mut_cout.unlock();
						realocate_out_driver_inside();
						break;
					}
					cout << "Введите объём топлива: "; cin >> amount;
					tank.fill(amount);
				}
				else
				{
					mut_cout.lock();

					GetConsoleScreenBufferInfo(hConsole, &info);
					SetConsoleCursorPosition(hConsole, cords.end);
					cout << "нужно заглушить двигатель и выйти из машины, у нас только самообслуживание." << endl;
					SetConsoleCursorPosition(hConsole, info.dwCursorPosition);

					mut_cout.unlock();
					realocate_out_driver_inside();
				}
			}
				break;
			case 'I':
			case 'i':
				if (driver_inside && !engine.started())startup();
				else if(driver_inside) shutdown();
				break;
			case Escape:
				shutdown();
				get_out();
				break;
			}
			static std::chrono::time_point<std::chrono::system_clock> last = std::chrono::system_clock::now();
			if (speed && std::chrono::system_clock::now() - last >= 1s)
			{
				speed -= 1;
				if (!speed)speed = 0;
			    engine.consumption_to_speed(speed);
				last = std::chrono::system_clock::now();
			}
			if (tank.get_fuel_level() == 0 && engine.started())shutdown();
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
		realocate_out_driver_inside();
		while(driver_inside)
		{
			if(cashe.tank_fuel_level != tank.get_fuel_level())
			{
				mut_cout.lock();
				SetConsoleCursorPosition(hConsole, cords.fuel_level);
				cout << (cashe.tank_fuel_level = tank.get_fuel_level());
				mut_cout.unlock();
				
			}
			if(cashe.low_fuel != tank.get_fuel_level() < 5)
			{
				mut_cout.lock();
				SetConsoleCursorPosition(hConsole, cords.low_fuel);
				if ((cashe.low_fuel = tank.get_fuel_level() < 5))
				{
					SetConsoleTextAttribute(hConsole, 0x4F);
					cout << " LOW FUEL ";
					SetConsoleTextAttribute(hConsole, 0x07);
				}
				mut_cout.unlock();
			}
			if(cashe.car_speed != speed)
			{
				mut_cout.lock();
				SetConsoleCursorPosition(hConsole, cords.car_speed);
				cout << (cashe.car_speed = speed) << "   ";
				mut_cout.unlock();
			}
			if(cashe.engine_is_started != engine.started())
			{
				mut_cout.lock();
				SetConsoleCursorPosition(hConsole, cords.engine_is_started);
				if ((cashe.engine_is_started = engine.started()))
				{
					SetConsoleTextAttribute(hConsole, 0x0A);
					cout << " STARTED ";
					SetConsoleTextAttribute(hConsole, 0x07);
				}
				else
				{
					SetConsoleTextAttribute(hConsole, 0x04);
					cout << " STOPED ";
					SetConsoleTextAttribute(hConsole, 0x07);
				}
				mut_cout.unlock();
			}
			if(cashe.engine_consumption != engine.get_consumption_per_second())
			{
				mut_cout.lock();
				SetConsoleCursorPosition(hConsole, cords.engine_consumption);
				cout << (cashe.engine_consumption = engine.get_consumption_per_second()) << "  ";
				mut_cout.unlock();
			}
		}
		/*while (driver_inside)
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
			cout << "Speed: \t" << speed << " km/h.\n";
			cout << "Engine is ";
			if(engine.started())
			{
				SetConsoleTextAttribute(hConsole, 0x0A);
				cout << " STARTED ";
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			else
			{
				SetConsoleTextAttribute(hConsole, 0x04);
				cout << " STOPED ";
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			cout << endl;
			cout << "Engine consumption: \t" << engine.get_consumption_per_second() << endl;
			std::this_thread::sleep_for(100ms);
		}*/
	}
private:
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	struct
	{
		COORD fuel_level;
		COORD low_fuel;
		COORD car_speed;
		COORD engine_is_started;
		COORD engine_consumption;
		COORD end;
	}cords;
	struct
	{
		double tank_fuel_level;
		bool low_fuel;
		int car_speed;
		bool engine_is_started;
		double engine_consumption;
	}cashe{0,0,0,0,0};
	std::mutex mut_cout;
	void realocate_out_driver_inside()
	{
		if (!driver_inside)return;
		mut_cout.lock();

		system("CLS");
		cout << "Fuel level: ";
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.fuel_level = info.dwCursorPosition;
		cout << cashe.tank_fuel_level << "\t\tliters.\t";

		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.low_fuel = info.dwCursorPosition;
		if (cashe.low_fuel)
		{
			SetConsoleTextAttribute(hConsole, 0x4F);
			cout << " LOW FUEL ";
			SetConsoleTextAttribute(hConsole, 0x07);
		}
		cout << endl;

		cout << "Speed: \t    ";
		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.car_speed = info.dwCursorPosition;
		cout << cashe.car_speed << "\t\tkm/h.\n";

		cout << "Engine is ";
		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.engine_is_started = info.dwCursorPosition;
		if (cashe.engine_is_started)
		{
			SetConsoleTextAttribute(hConsole, 0x0A);
			cout << " STARTED ";
			SetConsoleTextAttribute(hConsole, 0x07);
		}
		else
		{
			SetConsoleTextAttribute(hConsole, 0x04);
			cout << " STOPED ";
			SetConsoleTextAttribute(hConsole, 0x07);
		}
		cout << endl;

		cout << "Engine consumption: \t";
		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.engine_consumption = info.dwCursorPosition;
		cout << cashe.engine_consumption << "\tliters/sec." << endl;
		GetConsoleScreenBufferInfo(hConsole, &info);
		cords.end = info.dwCursorPosition;

		mut_cout.unlock();
	};
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