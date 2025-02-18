#pragma once

#include <functional>
#include <vector>
#include <map>

class UI;

struct Timer
{
	void NewFrame(float dt);
	void AddAction(uint16_t period, std::function<void()> e);
	void Reset();

	uint16_t		   limit = 1400;
	std::atomic<float> sum = 0.0f;
	std::vector<float> stamps;

	std::multimap<uint16_t, std::pair<std::function<void()>, uint32_t>> eventlist;
};