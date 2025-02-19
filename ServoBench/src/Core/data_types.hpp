#pragma once

#define S2WS(str) std::wstring(str.begin(), str.end())

#include <vector>
#include <string>
#include <queue>

using buffer_t    = std::vector<int8_t>;
using task_list_t = std::queue<std::string>;

// Структура отказов
struct fault_t
{
	union
	{
		uint8_t time_out : 1;
	} bits;

	uint8_t value;
};

// Структура лога
struct log_t
{
	log_t()
	{
		name.reserve(20);
		period = 100.0f;
	}

	std::string name;
	float       period;
};