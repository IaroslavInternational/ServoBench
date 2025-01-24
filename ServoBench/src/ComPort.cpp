#include "ComPort.hpp"

#include "CoreLog.hpp"
#include "Core/data_types.hpp"

#define CHECK_ERROR(expr, status, comment) \
if(expr) \
{					\
	status = false; \
	LOG(comment);   \
} \
else \
{	 \
	status = true;	\
}

ComPort::~ComPort()
{
	Close();
}

void ComPort::Open(const std::string& portName, uint32_t speed, uint8_t byte_size, uint8_t stop_bytes, uint8_t parity)
{	
	LOG_H("ComPort");

	if (isActive)
	{
		LOG("Port " + name + " opened! Reopen on " + portName + "...\n");
		Close();
	}

	name = portName;
	this->speed = speed;
	this->byte_size = byte_size;
	this->parity = parity;

	LOG("Port ");
	LOG(portName);
	LOG(" init:\n");

	// Open port
	Serial = CreateFile(S2WS(portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	CHECK_ERROR(Serial == INVALID_HANDLE_VALUE,         isActive, "Error: Unknown handle error\n");
	CHECK_ERROR(GetLastError() == ERROR_FILE_NOT_FOUND, isActive, "Error: Serial Port not found\n");

	DCB dcbSerialParam = { 0 };
	dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

	CHECK_ERROR(!GetCommState(Serial, &dcbSerialParam), isActive, "Error: Get params failed\n");

	dcbSerialParam.BaudRate = speed;
	dcbSerialParam.ByteSize = byte_size;
	dcbSerialParam.StopBits = stop_bytes;
	dcbSerialParam.Parity   = parity;

	CHECK_ERROR(!SetCommState(Serial, &dcbSerialParam), isActive, "Error: Set params failed\n");

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	CHECK_ERROR(SetCommTimeouts(Serial, &timeouts) == 0, isActive, "Error: Set timeouts failed\n");

	LOG_T("Speed",      speed);
	LOG_T("Byte Size",  byte_size);
	LOG_T("Stop Bytes", stop_bytes);
	LOG_T("Parity",     parity);

	LOG_END();
}

void ComPort::Close()
{
	LOG_H("ComPort");

	if (!CloseHandle(Serial))
	{
		LOG("Error: Port cannot be closed\n");
	}
	else
	{
		LOG("Port " + name + " is closed\n");
		isActive = false;
	}

	LOG_END();
}

bool ComPort::IsOpen() const
{
	return isActive;
}

#undef CHECK_ERROR