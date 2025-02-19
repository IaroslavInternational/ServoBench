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
	void Render(float dt); // Рендер
private:
	void ShowLeftPanel();  // Рендер левой панели
	void ShowMainChart();  // Рендер главного графика
	void ShowTable();	   // Рендер таблицы
	void SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset);
private:
	std::list<int> getAvailablePorts();					    // Получить доступные порты
	void		   TryConnection(const std::string& name);  // Функция установки подключения (15 попыток) -- в отдельном потоке ConnectionThread
	void		   CloseConnection();						// Функция отключения порта -- в отдельном потоке ConnectionThread
	void		   ReceiveData();						    // Функция получения данных в бесконечном цикле -- в отдельном потоке RxThread
	void		   DataProc(buffer_t* pData);				// Функция обработки данных (из ReceiveData)
	void		   GetCmd();								// Функция парсинга комманд в бесконечном цикле -- в отдельном потоке CmdThread
private:
	// Функция добавления графика в таблицу
	template<typename T>
	void PlotTableSensor(Sensor<T>* sensor, const std::string& header, const std::string& units, const std::string& spec, uint16_t colors);

	template<typename T>
	T GetLast(const Sensor<T>& sensor);
private:
	void AddLog();
	void AddLogLine();
private:
	ComPort             port;		      // Порт
	std::mutex          mtx;		      // Mutex для управления tasks
	std::future<void>   ConnectionThread; // Асинхронный поток подключения к порту
	std::future<void>   RxThread;         // Асинхронный поток приёма данных
	std::future<void>   CmdThread;        // Асинхронный поток обработки данных
	std::array<bool, 3> selected;         // Выбранный сигнал для отрисовки
	task_list_t	        tasks;            // Список команд
private:
	std::atomic<bool> ThreadsAllowed = false; // Флаг прерывания потоков RxThread и CmdThread
	Timer		      timer;				  // Таймер
private:
	Sensor<float> temperature; // Датчик температуры
	Sensor<float> current;	   // Датчик тока
	Sensor<float> voltage;	   // Датчик напряжения

	Sensor<float>*     choosen_sensor = nullptr; // Выбранный датчик для отрисовки на главный график
	std::vector<float> out_buffer;				 // Буфер данных для отрисовки данных на главный график
};
