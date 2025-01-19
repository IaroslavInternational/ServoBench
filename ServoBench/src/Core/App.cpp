#include "App.hpp"

App::App(const std::wstring& name, int x, int y)
	:
	wnd(name, x, y)
{
}

void App::Go()
{
	wnd.BeginFrame();
	ui.Render();
	wnd.EndFrame();
}
