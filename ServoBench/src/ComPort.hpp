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
	void Open(const std::string& portName, uint32_t speed = CBR_9600, uint8_t byte_size = 8, uint8_t stop_bytes = ONESTOPBIT, uint8_t parity = NOPARITY);
	void Close();
	void TxData(int8_t* data);
	buffer_t* RxData();
public:
	bool IsOpen() const;
	void SetBufferSize(uint64_t buffer_size);
	void ClearBuffer();
private:
	HANDLE Serial   = nullptr;
	bool   isActive = false;

	std::string name      = "NO PORT";
	uint32_t    speed     = CBR_9600;
	uint8_t     byte_size = ONESTOPBIT;
	uint8_t     parity    = NOPARITY;

	DWORD		     	SentSize;
	DWORD		     	readSize;
	uint64_t     	    buffer_size;
	buffer_t            buffer;
};

