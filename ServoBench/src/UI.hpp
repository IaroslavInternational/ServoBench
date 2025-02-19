#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <array>
#include <future>
#include <mutex>
#include <functional>

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
	void Render(float dt); // ������
private:
	void ShowLeftPanel();  // ������ ����� ������
	void ShowMainChart();  // ������ �������� �������
	void ShowTable();	   // ������ �������
	void SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset);
private:
	std::list<int> getAvailablePorts();					    // �������� ��������� �����
	void		   TryConnection(const std::string& name);  // ������� ��������� ����������� (15 �������) -- � ��������� ������ ConnectionThread
	void		   CloseConnection();						// ������� ���������� ����� -- � ��������� ������ ConnectionThread
	void		   ReceiveData();						    // ������� ��������� ������ � ����������� ����� -- � ��������� ������ RxThread
	void		   DataProc(buffer_t* pData);				// ������� ��������� ������ (�� ReceiveData)
	void		   GetCmd();								// ������� �������� ������� � ����������� ����� -- � ��������� ������ CmdThread
private:
	// ������� ���������� ������� � �������
	template<typename T>
	void PlotTableSensor(Sensor<T>* sensor, const std::string& header, const std::string& units, const std::string& spec, uint16_t colors);

	template<typename T>
	T GetLast(const Sensor<T>& sensor);
private:
	void AddLog();
	void AddLogLine();
private:
	ComPort             port;		      // ����
	std::mutex          mtx;		      // Mutex ��� ���������� tasks
	std::future<void>   ConnectionThread; // ����������� ����� ����������� � �����
	std::future<void>   RxThread;         // ����������� ����� ����� ������
	std::future<void>   CmdThread;        // ����������� ����� ��������� ������
	std::array<bool, 3> selected;         // ��������� ������ ��� ���������
	task_list_t	        tasks;            // ������ ������
private:
	std::atomic<bool> ThreadsAllowed = false; // ���� ���������� ������� RxThread � CmdThread
	Timer		      timer;				  // ������
private:
	Sensor<float> temperature; // ������ �����������
	Sensor<float> current;	   // ������ ����
	Sensor<float> voltage;	   // ������ ����������

	Sensor<float>*     choosen_sensor = nullptr; // ��������� ������ ��� ��������� �� ������� ������
	std::vector<float> out_buffer;				 // ����� ������ ��� ��������� ������ �� ������� ������
};
