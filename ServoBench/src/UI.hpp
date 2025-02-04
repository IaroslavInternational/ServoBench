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
	ComPort port;				 // ����
	std::mutex mtx;
	std::future<void> RxThread;  // ����������� ����� ����� ������
	std::future<void> CmdThread;  // ����������� ����� ��������� ������
	std::vector<bool> selected;  // ��������� ������ �����
	task_list_t tasks;			 // ������ ������
private:
	std::vector<int16_t> temperature = {};
};
