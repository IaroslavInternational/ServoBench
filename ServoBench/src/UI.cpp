#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/lib.hpp"
#include "../imgui/imgui_internal.h"

#include <sstream>
#include <fstream>
#include <chrono>

#pragma execution_character_set("utf-8")  // Для отображения на русском языке

using namespace std::literals::chrono_literals;

namespace ImGui
{
	bool SliderFloatWithSteps(const char* label, float* v, float v_min, float v_max, float v_step, const char* display_format)
	{
		if (!display_format)
			display_format = "%.3f";

		char text_buf[64] = {};
		ImFormatString(text_buf, IM_ARRAYSIZE(text_buf), display_format, *v);

		// Map from [v_min,v_max] to [0,N]
		const int countValues = int((v_max - v_min) / v_step);
		int v_i = int((*v - v_min) / v_step);
		const bool value_changed = SliderInt(label, &v_i, 0, countValues, text_buf);

		// Remap from [0,N] to [v_min,v_max]
		*v = v_min + float(v_i) * v_step;
		return value_changed;
	}
}

UI::UI()
	:
	temperature("Температура", "T", -40, 40, 20),
	current("Ток", "C", 0, 5, 20),
	voltage("Напряжение", "V", 0, 20, 20),
	selected({false, false, false})
{
	ImGui::GetStyle().WindowBorderSize = 0.0f;
	ImGui::GetStyle().TabBorderSize    = 1.0f;
	ImGui::GetStyle().TabBarBorderSize = 1.0f;
	ImGui::GetStyle().FrameRounding    = 8.0f;
	ImGui::GetStyle().GrabRounding     = 5.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg]       = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_FrameBg]        = ImVec4(0.07f, 0.07f, 0.07f, 0.54f);
	colors[ImGuiCol_TitleBg]        = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive]  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg]      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabSelected]    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabHovered]     = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Tab]            = ImVec4(0.02f, 0.02f, 0.02f, 0.86f);
	colors[ImGuiCol_Button]         = ImVec4(0.55f, 0.47f, 0.03f, 0.91f);
	colors[ImGuiCol_ButtonHovered]  = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.18f, 0.19f, 0.40f);
	colors[ImGuiCol_FrameBgActive]  = ImVec4(0.24f, 0.26f, 0.29f, 0.40f);
	colors[ImGuiCol_CheckMark]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Button]         = ImVec4(0.34f, 0.06f, 0.06f, 0.91f);
	colors[ImGuiCol_ButtonActive]   = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
	colors[ImGuiCol_Header]         = ImVec4(0.42f, 0.13f, 0.13f, 0.31f);
	colors[ImGuiCol_HeaderHovered]  = ImVec4(0.22f, 0.05f, 0.05f, 0.80f);
	colors[ImGuiCol_HeaderActive]   = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PopupBg]        = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

	LOG_H("UI");
	LOG("Colors are set\n");

	LOG_END();
}

void UI::Render(float dt)
{
	//ImPlot::ShowDemoWindow();
	ImGui::ShowDemoWindow();
	SetPanelSizeAndPosition(0, 0.2f, 1.0f, 0.0f, 0.0f);
	ShowLeftPanel();
	
	{
		if (ImGui::Begin("Debug wnd"))
		{
			static signed char txt[20];
			ImGui::InputText("Data", (char*)txt, 20);

			if (ImGui::Button("Send"))
			{				
				port.TxData(txt);
			}

			ImGui::SliderInt("Limit", (int*)&timer.limit, 450, 8000);

			static uint32_t period = 1;
			ImGui::SliderInt("Period sec", (int*)&period, 1, 10);
			if (ImGui::Button("Create Log Event"))
			{
				//timer.AddAction(period, std::bind(&UI::AddLog, this));
				//timer.StartEvents();
			}
		}

		ImGui::End();
	}

	SetPanelSizeAndPosition(0, 0.5f, 0.5f, 0.2f, 0.45f);
	ShowTable();

	SetPanelSizeAndPosition(0, 0.8f, 0.5f, 0.2f, 0.0f);
	ShowMainChart();

	// Получение меток времени
	if (port.IsOpen() && ThreadsAllowed)
	{
		timer.NewFrame(dt);

		// Синхронизация размеров буферов
		if (out_buffer.size() > timer.limit)
		{
			out_buffer.erase(out_buffer.begin());
		}

		// Обновление размеров буферов
		if (timer.limit != out_buffer.size())
		{
			if (out_buffer.size() > timer.limit)
			{
				out_buffer.erase(out_buffer.begin(), out_buffer.begin() + (out_buffer.size() - timer.limit));
				timer.stamps.erase(timer.stamps.begin(), timer.stamps.begin() + (timer.stamps.size() - out_buffer.size()));
			}
		}

		// Добавление новых данных
		if (choosen_sensor != nullptr)
		{
			if (choosen_sensor->GetSize() > 0)
				out_buffer.emplace_back(choosen_sensor->GetLast());
			else
				out_buffer.emplace_back(0.0f);
		}
	}
}

void UI::ShowLeftPanel()
{
	if (ImGui::Begin("MainBar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTabBar("Main bar"))
		{
			if (ImGui::BeginTabItem("Лог"))
			{
				static log_t log_data;
				static bool flag = false;

				if (ImGui::InputText("Имя", (char*)log_data.name.c_str(), log_data.name.capacity()))
				{
					flag = false;
					log_data.name = log_data.name.c_str();
				}

				if (flag)
				{
					ImGui::TextColored({ 1.0f,0.0f,0.0f,1.0f }, "Введите имя файла");
				}

				ImGui::SliderFloatWithSteps("Период, мс", &log_data.period, 100.0f, 10000.0f, 100.0f, "%.0f");

				if (ImGui::Button("Создать лог"))
				{
					if (log_data.name.empty())
					{
						flag = true;
					}
					else
					{
						InitLog(log_data);
						timer.AddAction(log_data, std::bind(&UI::AddLogThread, this, log_data));
					}
				}

				if (ImGui::Button("Запустить"))
				{
					timer.StartEvents();
				}
				
				ImGui::Text("Созданные логи:");
				for (auto& [period, event_data] : timer.eventlist)
				{
					ImGui::BulletText(("Лог " + std::to_string(int(period * 1000.0f)) + "ms").c_str());
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Подключение"))
			{
				auto ports = getAvailablePorts();
				std::vector<std::string> names;

				for (auto& p : ports)
				{
					names.emplace_back(std::string("COM") + std::to_string(p) + '\0');
				}

				static int32_t item_current = 0;
				ImGui::Combo("Порты", &item_current,
					[](void* vec, int idx, const char** out_text) 
					{
						std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
							if (idx < 0 || idx >= vector->size()) 
								return false;
						*out_text = vector->at(idx).c_str();
						return true;
					}, reinterpret_cast<void*>(&names), names.size());

				if (fault.bits.time_out)
				{
					ImGui::Text(("Не удалось подключиться к " + names[item_current]).c_str());
					return;
				}
				else
				{
					ImGui::Text(("Подключено к " + port.GetName()).c_str());
				}

				if (!port.IsOpen())
				{
					if (ImGui::Button("Подключить"))
					{				
						ConnectionThread = std::async(std::launch::async, &UI::TryConnection, this, names[item_current]);
					}
				}
				else
				{
					if (ImGui::Button("Закрыть"))
					{
						ConnectionThread = std::async(std::launch::async, &UI::CloseConnection, this);
					}
				}
				
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void UI::ShowMainChart()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.00f));
	if (ImGui::Begin("Data", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (timer.stamps.size() > 0)
			ImPlot::SetNextAxesLimits(timer.stamps[0], timer.stamps.back(), 0.0, 1.0, ImPlotCond_Always);
		if (timer.stamps.size() > 0 && choosen_sensor != nullptr)
			ImPlot::SetNextAxesLimits(timer.stamps[0], timer.stamps.back(), choosen_sensor->GetMin(), choosen_sensor->GetMax(), ImPlotCond_Always);
		
		if (ImPlot::BeginPlot(port.GetName().c_str()))
		{
			if (choosen_sensor != nullptr && !out_buffer.empty())
			{				
				ImPlot::PlotLine((choosen_sensor->GetName()).c_str(), timer.stamps.data(), out_buffer.data(), out_buffer.size());
			}

			ImPlot::EndPlot();
		}

	}
	ImGui::PopStyleColor();

	ImGui::End();
}

void UI::ShowTable()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.00f));
	if (ImGui::Begin("Data Metrics", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		static const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;

		static uint16_t colors = 0;

		if (ImGui::BeginTable("##table", 4, flags, ImVec2(-1, 0)))
		{
			ImGui::TableSetupColumn("Показатель", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Значение", ImGuiTableColumnFlags_WidthFixed, 75.0f);
			ImGui::TableSetupColumn("Сигнал");
			ImGui::TableSetupColumn("Вывести");
			ImGui::TableHeadersRow();
			ImPlot::PushColormap(ImPlotColormap_Viridis);

			// Не менять цвета
			colors = 0;
			PlotTableSensor(&temperature, "Температура", "°C", "%.2f", colors);

			colors = 1;
			PlotTableSensor(&current, "Ток", "A", "%.2f", colors);

			colors = 2;
			PlotTableSensor(&voltage, "Напряжение", "V", "%.2f", colors);

			ImPlot::PopColormap();
			ImGui::EndTable();
		}
	}
	ImGui::PopStyleColor();

	ImGui::End();
}

void UI::SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset)
{
	ImGuiIO& io = ImGui::GetIO();

	float MenuHeight = 0.0f;
	ImVec2 DispSize = io.DisplaySize;

	float PanelW = round(DispSize.x * width);
	float PanelH = DispSize.y * height;

	ImVec2 PanelSize = ImVec2(
		PanelW,
		PanelH
	);

	ImVec2 PanelPos = ImVec2(
		(corner & 1) ? DispSize.x + round(DispSize.x * x_offset) : round(DispSize.x * x_offset),
		(corner & 2) ? DispSize.y + MenuHeight + DispSize.y * y_offset : MenuHeight + DispSize.y * y_offset
	);

	ImVec2 PanelPivot = ImVec2(
		(corner & 1) ? 1.0f : 0.0f,
		(corner & 2) ? 1.0f : 0.0f
	);

	ImGui::SetNextWindowPos(PanelPos, ImGuiCond_Always, PanelPivot);
	ImGui::SetNextWindowSize(PanelSize);
}

std::list<int> UI::getAvailablePorts()
{
	wchar_t lpTargetPath[5000];
	std::list<int> portList;

	for (uint16_t i = 0; i < 255; i++)
	{
		std::wstring str = L"COM" + std::to_wstring(i);
		DWORD res = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		if (res != 0) 
		{
			portList.push_back(i);
		}
	}

	return portList;
}

void UI::TryConnection(const std::string& name)
{
	fault.bits.time_out = 0;

	static uint8_t cnt = 0;
	const  uint8_t max_attemps = 15;

	signed char connectionCode[] = "go";
	
	if (!port.Open(name, 38400u))
	{
		LOG("TRY FAILED");
		return;
	}
	
	while (port.RxData() == nullptr)
	{
		port.TxData(connectionCode);
		std::this_thread::sleep_for(10ms);

		if (++cnt >= max_attemps)
		{
			fault.bits.time_out = 1;
		}
	}
	
	ThreadsAllowed = true;
	
	RxThread  = std::async(std::launch::async, &UI::ReceiveData, this);
	CmdThread = std::async(std::launch::async, &UI::GetCmd,      this);
}

void UI::CloseConnection()
{
	ThreadsAllowed = false;

	// Каждые 50мс ожидаем завершения потоков и закрываем порт
	while (!lib::IsThreadTerminated(RxThread) && !lib::IsThreadTerminated(CmdThread))
	{
		std::this_thread::sleep_for(50ms);
	}

	port.TxData((int8_t*)"out");
	port.Close();	
	
	timer.Reset();
}

void UI::ReceiveData()
{
	while(!false)
	{
		if (!ThreadsAllowed)
			return;

		DataProc(port.RxData());
	}
}

void UI::DataProc(buffer_t* pData)
{
	if (pData == nullptr)
	{
		return;
	}

	static const std::string splitter = "\r\n";
	std::string data(pData->begin(), pData->end());
	
	while (data.find(splitter) != -1)
	{
		tasks.emplace(data.begin(), data.begin() + data.find(splitter));
		data = std::string(data.begin() + data.find(splitter) + splitter.size(), data.end());
	}
}

void UI::GetCmd()
{
	std::string cmd;
	timer.sum = 0.0f;

	while (!false)
	{
		if (!ThreadsAllowed)
			return;

		if (!tasks.empty())
		{
			mtx.lock();
			cmd = tasks.front();
			
			if (temperature.Add(cmd))
			{	
				tasks.pop();
			}
			else if (current.Add(cmd))
			{
				tasks.pop();
			}
			else if (voltage.Add(cmd))
			{
				tasks.pop();
			}
			else
			{
				tasks.pop(); // force pop of uncorrect cmd
			}
			mtx.unlock();
		}		
	}
}

template<typename T>
void UI::PlotTableSensor(Sensor<T>* sensor, const std::string& header, const std::string& units, const std::string& spec, uint16_t colors)
{
	if (!sensor->IsEmpty())      
	{
		ImGui::TableNextRow(); 
		ImGui::TableSetColumnIndex(0); 
		ImGui::Text(header.c_str()); 
		ImGui::TableSetColumnIndex(1); 
		ImGui::Text((spec + " " + units).c_str(), sensor->GetLast()); 
		ImGui::TableSetColumnIndex(2); 
		
		ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0)); 
		ImPlot::SetNextAxesLimits(0, (double)sensor->GetLimit(), (double)sensor->GetMin(), (double)sensor->GetMax(), ImPlotCond_Always);
		if (ImPlot::BeginPlot(("##" + header).c_str(), ImVec2(-1, 35), ImPlotFlags_CanvasOnly))
		{
			ImPlot::SetNextLineStyle(ImPlot::GetColormapColor(colors)); 
			ImPlot::SetNextFillStyle(ImPlot::GetColormapColor(colors), 0.25); 
			ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations); 
			ImPlot::PlotShaded(("##" + header + "_s_data").c_str(), sensor->Get(), (double)sensor->GetSize(), -10000.0, 1, 0);
			
			ImPlot::SetNextLineStyle(ImPlot::GetColormapColor(colors)); 
			ImPlot::SetNextFillStyle(ImPlot::GetColormapColor(colors), 0.25); 
			ImPlot::PlotLine(("##" + header + "_data").c_str(), sensor->Get(), (double)sensor->GetSize(), 1, 0);
			ImPlot::EndPlot(); 
		}

		ImGui::TableSetColumnIndex(3);
		ImGui::PushID(("##" + header + "_box").c_str());
		if (ImGui::Checkbox("", &selected[colors]))
		{
			if (selected[colors])
			{
				choosen_sensor = sensor;

				memset(selected.data(), false, selected.size());
				selected[colors] = true;
			}
			else
			{
				choosen_sensor = nullptr;

				memset(selected.data(), false, selected.size());
				selected[colors] = false;
			}

			out_buffer.clear();
			out_buffer.resize(timer.limit);

		}

		ImGui::PopID();
	}
}

template<typename T>
T UI::GetLast(const Sensor<T>& sensor)
{
	if (!sensor.IsEmpty())
	{
		return sensor.GetLast();
	}
}

void UI::AddLogThread(const log_t& data)
{
	auto thread = std::async(std::launch::async, &UI::AddLogLine, this, data.name);
}

void UI::AddLogLine(const std::string& filename)
{
	LOG_H("UI");
	LOG("Adding log in " + filename + "\n");

	std::ostringstream oss;
	
	std::string stamp = std::to_string(timer.stamps.back());
	std::replace(stamp.begin(), stamp.end(), '.', ',');

	oss << stamp << "\t" << GetLast(temperature) << "\t" << GetLast(current) << "\t" << GetLast(voltage);

	std::ofstream out;
	out.open(filename + ".txt", std::ios::app);

	if (out.is_open())
	{
		out << oss.str() << std::endl;
	}

	out.close();

	LOG_END();
}

void UI::InitLog(const log_t& log_data) const
{
	LOG_H("UI");
	LOG("Log file init " + log_data.name + "\n");

	remove((log_data.name + ".txt").c_str());

	std::ostringstream oss;
	oss << "Файл отчёта " << log_data.name << " Servo Bench\n";
	oss << "Дата: " << std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() } << "\n\n";
	oss << "Время\t" << "Температура\t" << "Ток\t" << "Напряжение\t\n";

	std::ofstream out;
	out.open(log_data.name + ".txt");

	if (out.is_open())
	{
		out << oss.str() << std::endl;
	}

	out.close();

	LOG_END();
}