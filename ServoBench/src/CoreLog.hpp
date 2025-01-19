#pragma once

#ifdef _DEBUG

#include <iostream>

#define LOG(data) std::cout << data
#define LOG_H(header) LOG(">>"); LOG(header); LOG(" module\n");
#define LOG_END() LOG("\n\n")

#else
#define __FILENAME__
#define LOG(data) 
#define LOG_H(header) 
#define LOG_END()
#endif