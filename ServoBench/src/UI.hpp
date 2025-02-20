#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/data_types.hpp"
#include "ComPort.hpp"

#include <list>
#include <vector>
#include <queue>
#include <array>
#include <future>
#include <mutex>
#include <functional>

#include "Sensor.hpp"

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
	void AddLogThread(const log_t& data);
	void AddLogLine(const std::string& filename);
	void InitLog(const log_t& log_data) const;
private:
	ComPort             port;		      // Порт
	std::mutex          mtx;		      // Mutex для управления tasks
	std::future<void>   ConnectionThread; // Асинхронный поток подключения к порту
	std::future<void>   RxThread;         // Асинхронный поток приёма данных
	std::future<void>   CmdThread;        // Асинхронный поток обработки данных
	std::array<bool, 3> selected;         // Выбранный сигнал для отрисовки
	task_list_t	        tasks;            // Список команд
private:
	fault_t			  fault;				  // Структура состояния подключения к порту
	std::atomic<bool> ThreadsAllowed = false; // Флаг прерывания потоков RxThread и CmdThread
	Timer		      timer;				  // Таймер
	std::map<float, std::string> match_log;
private:
	Sensor<float> temperature; // Датчик температуры
	Sensor<float> current;	   // Датчик тока
	Sensor<float> voltage;	   // Датчик напряжения

	Sensor<float>*     choosen_sensor = nullptr; // Выбранный датчик для отрисовки на главный график
	std::vector<float> out_buffer;				 // Буфер данных для отрисовки данных на главный график
};
