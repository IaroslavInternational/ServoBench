#pragma once

#include "Core/Window.hpp"
#include "Core/data_types.hpp"

#include <string>
#include <vector>

#include <fileapi.h>
#include <atlstr.h>

class ComPort
{
public:
	ComPort() = default;
	~ComPort();
public:
	bool Open(const std::string& portName, uint32_t speed = CBR_9600, uint8_t byte_size = 8, uint8_t stop_bytes = ONESTOPBIT, uint8_t parity = NOPARITY);
	void Close();
	void TxData(int8_t* data);
	buffer_t* RxData();
public:
	bool IsOpen() const;
	void SetBufferSize(uint64_t buffer_size);
	void ClearBuffer();
	std::string GetName() const;
private:
	HANDLE Serial   = nullptr;
	bool   isActive = false;

	std::string name      = "NO PORT";
	uint32_t    speed     = CBR_9600;
	uint8_t     byte_size = ONESTOPBIT;
	uint8_t     parity    = NOPARITY;

	DWORD    sentSize    = 0;
	DWORD    readSize    = 0;
	uint64_t buffer_size = 64;
	buffer_t buffer;
};

