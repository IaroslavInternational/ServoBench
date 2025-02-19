#pragma once

#include <functional>
#include <vector>
#include <map>
#include <future>

struct Timer
{
	void NewFrame(float dt);
	void AddAction(uint16_t period, std::function<void()> e);
	void Reset();
	void StartEvents();
	void ProcEvent();

	uint16_t		   limit = 1400;
	std::atomic<float> sum = 0.0f;
	std::vector<float> stamps;

	std::multimap<uint16_t, std::pair<std::function<void()>, uint32_t>> eventlist;
	std::future<void> thread;
	std::atomic<bool> IsTerminate = false;
};