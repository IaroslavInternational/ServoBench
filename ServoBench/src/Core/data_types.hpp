#pragma once

#define S2WS(str) std::wstring(str.begin(), str.end())

#include <vector>
#include <queue>

using buffer_t    = std::vector<int8_t>;
using task_list_t = std::queue<std::string>;