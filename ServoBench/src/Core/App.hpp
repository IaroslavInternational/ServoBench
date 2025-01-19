#pragma once

#include "Window.hpp"
#include "../UI.hpp"

class App
{
public:
	App(const std::wstring& name, int x, int y);
public:
	void Go();
private:
	Window wnd;
	UI ui;
};

