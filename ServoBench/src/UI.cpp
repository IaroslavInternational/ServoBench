#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/data_types.hpp"

#pragma execution_character_set("utf-8")  // Для отображения на русском языке

/* CODE GEN */

#define PLOT_SENSOR(sensor, units) \
if (!sensor.IsEmpty())      \
{\
	ImPlot::PlotLine((sensor.GetName() + " " + units).c_str(), sensor.Get(), sensor.GetLimit());\
}

/************/

using namespace std::literals::chrono_literals;

UI::UI()
	:
	temperature("Температура", "T"),
	current("Ток", "C"),
	voltage("Напряжение", "V")
{
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

void UI::Render()
{
	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();
	SetPanelSizeAndPosition(0, 0.2f, 1.0f, 0.0f, 0.0f);
	ShowLeftPanel();

	SetPanelSizeAndPosition(0, 0.8f, 0.5f, 0.2f, 0.0f);
	ShowMainChart();
	
	{
		static uint64_t i = 0;
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
	}
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

				if (ImGui::Button("Подключить"))
				{
					ConnectionThread = std::async(std::launch::async, &UI::TryConnection, this, names[item_current]);
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
		ImPlot::SetNextAxesLimits(0.0, (double)temperature.GetSize() - 1, 0.0, 30.0, ImPlotCond_Always);
		if (ImPlot::BeginPlot("Данные"))
		{
			PLOT_SENSOR(temperature, "t °C");
			PLOT_SENSOR(current, "A");
			PLOT_SENSOR(voltage, "V");

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
	}
	
	RxThread = std::async(std::launch::async, &UI::ReceiveData, this);
	CmdThread = std::async(std::launch::async, &UI::GetCmd, this);
}

void UI::ReceiveData()
{
	while(!false)
	{
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

	while (!false)
	{
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
