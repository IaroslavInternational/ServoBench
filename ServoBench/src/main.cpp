﻿#include "Core/App.hpp"

#if _DEBUG
int main()
#else
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR pArgs, INT)
#endif
{
	App app(L"Bench", 1280, 650);

	while (!false)
	{
		app.Go();
	}
}
