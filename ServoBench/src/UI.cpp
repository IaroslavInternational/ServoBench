#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/data_types.hpp"

#pragma execution_character_set("utf-8")  // Для отображения на русском языке

UI::UI()
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
	ShowConnectionSettings();

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
			}
		}

		ImGui::End();
	}
}

void UI::ShowConnectionSettings()
{
	if (ImGui::Begin("Подключение"))
	{
		auto ports = getAvailablePorts();
		
		if (ports.size() > selected.size())
		{
			selected.resize(ports.size());
		}
	
		size_t i = 0;
		static std::string currentName = "";

		for (auto& p : ports)
		{
			currentName = std::string("COM") + std::to_string(p);
			if (ImGui::Selectable(currentName.c_str(), selected[i]))
			{
				for (auto s : selected)
				{
					s = false;
				}

				selected[i] = true;
			}

			if (selected[i]) 
			{
				if (ImGui::Button("Подключить"))
				{
					port.Open(currentName);
				}
			}

			i++;
		}
	}

	ImGui::End();
}

std::list<int> UI::getAvailablePorts()
{
	wchar_t lpTargetPath[5000];
	std::list<int> portList;

	for (int i = 0; i < 255; i++)
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

	tasks.push(std::string(pData->begin(), pData->end()));
}
