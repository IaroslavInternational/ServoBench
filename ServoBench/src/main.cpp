#include "Core/App.hpp"

int main()
{
	App app(L"Bench", 1280, 650);

	while (!false)
	{
		app.Go();
	}
}
