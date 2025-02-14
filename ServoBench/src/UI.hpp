#pragma once

#include "Core/Window.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <future>
#include <mutex>

#include "Sensor.hpp"

struct 
{
	union 
	{
		uint8_t time_out : 1;
	} bits;

	uint8_t value;
} fault;

struct time_ctrl
{
	void NewFrame(float dt)
	{
		sum += dt;
	}

	void Stamp()
	{
		if (stamps.size() > limit)
		{
			stamps.erase(stamps.begin());
		}

		stamps.emplace_back(sum);
	}

	uint16_t		   limit = 450;
	std::atomic<float> sum   = 0.0f;
	std::vector<float> stamps;
};

class UI
{
public:
	UI();
public:
	void Render(float dt);
private:
	void ShowLeftPanel();
	void ShowMainChart();
	void SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset);
private:
	std::list<int> getAvailablePorts();
	void		   TryConnection(const std::string& name);
	void		   CloseConnection();
	void		   ReceiveData();
	void		   DataProc(buffer_t* pData);
	void		   GetCmd();
	template<typename T>
	bool		   IsThreadTerminated(std::future<T>& t);
private:
	ComPort port;				         // Порт
	std::mutex mtx;				         // Mutex для управления tasks
	std::future<void> ConnectionThread;  // Асинхронный поток подключения к порту
	std::future<void> RxThread;          // Асинхронный поток приёма данных
	std::future<void> CmdThread;         // Асинхронный поток обработки данных
	std::vector<bool> selected;          // Выбранный индекс порта
	task_list_t	      tasks;             // Список команд
	std::atomic<bool> ThreadsAllowed = false;
	time_ctrl		  time_temperature;
private:
	Sensor<float> temperature;
	Sensor<float> current;
	Sensor<float> voltage;

	std::vector<float> test_buffer;
};
