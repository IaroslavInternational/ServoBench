#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/data_types.hpp"

#pragma execution_character_set("utf-8")  // Для отображения на русском языке

/* CODE GEN */

#define PLOT_SENSOR(sensor, units) \
if (!sensor.IsEmpty())      \
{\
	ImPlot::PlotLine((sensor.GetName() + " " + units).c_str(), (float*)sensor.Get(), time_stamps.data(), sensor.GetSize());\
}

#define PLOT_TABLE_SENSOR(sensor, header, units, spec) \
if (!sensor.IsEmpty())      \
{\
	ImGui::TableNextRow();\
	ImGui::TableSetColumnIndex(0);\
	ImGui::Text(header);\
	ImGui::TableSetColumnIndex(1);\
	ImGui::Text((std::string(spec) + " " + units).c_str(), sensor.GetLast());\
	ImGui::TableSetColumnIndex(2);\
	\
	ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));\
	ImPlot::SetNextAxesLimits(0, (double)sensor.GetLimit(), (double)sensor.GetMin(), (double)sensor.GetMax(), ImPlotCond_Always);\
	if (ImPlot::BeginPlot(("##" + std::string(header)).c_str(), ImVec2(-1, 35), ImPlotFlags_CanvasOnly))\
	{\
		ImPlot::SetNextLineStyle(ImPlot::GetColormapColor(colors));\
		ImPlot::SetNextFillStyle(ImPlot::GetColormapColor(colors), 0.25); \
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);\
		ImPlot::PlotShaded(("##" + std::string(header) + "_s_data").c_str(), sensor.Get(), sensor.GetSize(), -10000.0, 1, 0); \
\
		ImPlot::SetNextLineStyle(ImPlot::GetColormapColor(colors)); \
		ImPlot::SetNextFillStyle(ImPlot::GetColormapColor(colors), 0.25); \
		ImPlot::PlotLine(("##" + std::string(header) + "_data").c_str(),     sensor.Get(), sensor.GetSize(), 1, 0); \
		ImPlot::EndPlot();\
	}\
}\


/************/

using namespace std::literals::chrono_literals;

UI::UI()
	:
	temperature("Температура", "T", -40, 40),
	current("Ток", "C", 0, 5),
	voltage("Напряжение", "V", 0, 20)
{
	//ImPlot::PlotS
	ImGui::GetStyle().WindowBorderSize = 0.0f;
	ImGui::GetStyle().TabBorderSize = 1.0f;
	ImGui::GetStyle().TabBarBorderSize = 1.0f;
	ImGui::GetStyle().FrameRounding = 8.0f;
	ImGui::GetStyle().GrabRounding = 5.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.07f, 0.54f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.02f, 0.02f, 0.02f, 0.86f);
	colors[ImGuiCol_Button] = ImVec4(0.55f, 0.47f, 0.03f, 0.91f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.18f, 0.19f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.26f, 0.29f, 0.40f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.34f, 0.06f, 0.06f, 0.91f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.42f, 0.13f, 0.13f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.05f, 0.05f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

	LOG_H("UI");
	LOG("Colors are set\n");
	LOG_END();
}

void UI::Render(float dt)
{
	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();
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

			if (ImGui::Button("Rx"))
			{
				port.ClearBuffer();
				RxThread = std::async(std::launch::async, &UI::ReceiveData, this);
				CmdThread = std::async(std::launch::async, &UI::GetCmd, this);
			}
		}

		ImGui::End();

		SetPanelSizeAndPosition(0, 0.5f, 0.5f, 0.2f, 0.45f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.00f));
		if (ImGui::Begin("Data Metrics", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			static const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
				ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;

			static uint16_t colors = 0;

			if (ImGui::BeginTable("##table", 3, flags, ImVec2(-1, 0))) 
			{
				ImGui::TableSetupColumn("Показатель", ImGuiTableColumnFlags_WidthFixed, 75.0f);
				ImGui::TableSetupColumn("Значение", ImGuiTableColumnFlags_WidthFixed, 75.0f);
				ImGui::TableSetupColumn("Сигнал");
				ImGui::TableHeadersRow();
				ImPlot::PushColormap(ImPlotColormap_Viridis);

				colors = 0;
				PLOT_TABLE_SENSOR(temperature, "Температура", "°C", "%d");

				colors = 1;
				PLOT_TABLE_SENSOR(current, "Ток", "A", "%d");

				colors = 2;
				PLOT_TABLE_SENSOR(voltage, "Напряжение", "V", "%d");

				ImPlot::PopColormap();
				ImGui::EndTable();
			}
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}

	SetPanelSizeAndPosition(0, 0.8f, 0.5f, 0.2f, 0.0f);
	ShowMainChart();

	time_sum += dt;
}

void UI::ShowLeftPanel()
{
	if (ImGui::Begin("MainBar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTabBar("Main bar"))
		{
			if (ImGui::BeginTabItem("Подключение"))
			{
				auto ports = getAvailablePorts();
				std::vector<std::string> names;

				if (ports.size() > selected.size())
				{
					selected.resize(ports.size());
				}

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
		if (time_stamps.size() > 0)
			ImPlot::SetNextAxesLimits(time_stamps[0], time_stamps.back(), 0.0, 30.0, ImPlotCond_Always);

		ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
		if (ImPlot::BeginPlot(port.GetName().c_str()))
		{
			//PLOT_SENSOR(temperature, "t °C");
			if (!temperature.IsEmpty())      
			{				
				ImPlot::PlotLine((temperature.GetName() + " " + "t °C").c_str(), time_stamps.data(), temperature.Get(), temperature.GetSize());
			}
			//PLOT_SENSOR(current, "A");
			//PLOT_SENSOR(voltage, "V");

			ImPlot::EndPlot();
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

	port.Close();

	time_sum = 0.0f;
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
	time_sum = 0;

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
				if (time_stamps.size() > 50)
					time_stamps.erase(time_stamps.begin());

				time_stamps.push_back(time_sum);

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
