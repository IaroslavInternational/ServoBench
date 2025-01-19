#pragma once

#include "Core/Window.hpp"
#include <list>
#include <vector>

class UI
{
public:
	UI();
public:
	void Render();
private:
	void ShowConnectionSettings();
private:
	std::list<int> getAvailablePorts();
};
