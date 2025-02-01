#pragma once

#include "Core/Window.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <future>

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
private:
	ComPort port;
	std::future<void> RxThread;

	std::vector<bool> selected;

	task_list_t tasks;
};
