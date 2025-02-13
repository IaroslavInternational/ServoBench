#include "App.hpp"

App::App(const std::wstring& name, int x, int y)
	:
	wnd(name, x, y)
{
}

void App::Go()
{
	const auto old = timer;
	timer = std::chrono::steady_clock::now();
	const std::chrono::duration<float> frameTime = timer - old;
	
	wnd.BeginFrame();
	ui.Render(frameTime.count());
	wnd.EndFrame();
}
