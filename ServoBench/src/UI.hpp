#pragma once

#include "Core/Window.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <future>
#include <mutex>

#include "Sensor.hpp"

class UI
{
public:
	UI();
public:
	void Render();
private:
	void ShowLeftPanel();
	void ShowMainChart();
	void SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset);
private:
	std::list<int> getAvailablePorts();
	void		   TryConnection(const std::string& name);
	void		   ReceiveData();
	void		   DataProc(buffer_t* pData);
	void		   GetCmd();
private:
	ComPort port;				 // Порт
	std::mutex mtx;				 // Mutex для управления tasks
	std::future<void> ConnectionThread;  // Асинхронный поток подключения к порту
	std::future<void> RxThread;  // Асинхронный поток приёма данных
	std::future<void> CmdThread; // Асинхронный поток обработки данных
	std::vector<bool> selected;  // Выбранный индекс порта
	task_list_t tasks;			 // Список команд
private:
	Sensor<int16_t> temperature;
	Sensor<int16_t> current;
	Sensor<int16_t> voltage;
};
