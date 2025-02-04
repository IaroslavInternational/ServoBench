#pragma once

#include "Core/Window.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <future>
#include <mutex>

class UI
{
public:
	UI();
public:
	void Render();
private:
	void ShowConnectionSettings();
private:
	std::list<int> getAvailablePorts();
	void		   ReceiveData();
	void		   DataProc(buffer_t* pData);
	void		   GetCmd();
private:
	ComPort port;				 // Порт
	std::mutex mtx;
	std::future<void> RxThread;  // Асинхронный поток приёма данных
	std::future<void> CmdThread;  // Асинхронный поток обработки данных
	std::vector<bool> selected;  // Выбранный индекс порта
	task_list_t tasks;			 // Список команд
private:
	std::vector<int16_t> temperature = {};
};
