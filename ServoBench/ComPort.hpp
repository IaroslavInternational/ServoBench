#pragma once

#include <string>

class ComPort
{
public:
	ComPort(const std::string& portName, int speed);
public:
	bool Open();
	bool Close();
	bool TxData();
	bool RxData();
private:
	std::string name;
	int speed;
};

