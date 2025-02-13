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
private:
	ComPort port;				 // ����
	std::mutex mtx;				 // Mutex ��� ���������� tasks
	std::future<void> ConnectionThread;  // ����������� ����� ����������� � �����
	std::future<void> RxThread;  // ����������� ����� ����� ������
	std::future<void> CmdThread; // ����������� ����� ��������� ������
	std::vector<bool> selected;  // ��������� ������ �����
	task_list_t tasks;			 // ������ ������
	std::atomic<bool> ThreadsAllowed = false;
	std::atomic<float> time_sum = 0.0f;
private:
	Sensor<float>   temperature;
	Sensor<int16_t> current;
	Sensor<int16_t> voltage;
	std::vector<float> time_stamps;
};
