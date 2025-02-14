#include "ComPort.hpp"

#include "CoreLog.hpp"

#define CHECK_ERROR(expr, status, comment) \
if(expr) \
{					  \
	status = false;   \
	LOG_ERR(comment); \
} \
else \
{	 \
	status = true;	\
}

ComPort::~ComPort()
{
	Close();
}

bool ComPort::Open(const std::string& portName, uint32_t speed, uint8_t byte_size, uint8_t stop_bytes, uint8_t parity)
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
	Serial = CreateFile(S2WS(portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);
	
	CHECK_ERROR(Serial == INVALID_HANDLE_VALUE,         isActive, "Unknown handle error");
	CHECK_ERROR(GetLastError() == ERROR_FILE_NOT_FOUND, isActive, "Serial Port not found");

	DCB dcbSerialParam = { 0 };
	dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

	CHECK_ERROR(!GetCommState(Serial, &dcbSerialParam), isActive, "Get params failed");

	dcbSerialParam.BaudRate = speed;
	dcbSerialParam.ByteSize = byte_size;
	dcbSerialParam.StopBits = stop_bytes;
	dcbSerialParam.Parity   = parity;

	CHECK_ERROR(!SetCommState(Serial, &dcbSerialParam), isActive, "Set params failed");

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	CHECK_ERROR(SetCommTimeouts(Serial, &timeouts) == 0, isActive, "Set timeouts failed");

	SetBufferSize(100);

	LOG_T("Speed",       speed);
	LOG_T("Byte Size",   (uint16_t)byte_size);
	LOG_T("Stop Bytes",  (uint16_t)stop_bytes);
	LOG_T("Parity",      (uint16_t)parity);
	LOG_T("Buffer Size", buffer_size);

	LOG_END();

	// Clear internal buffer
	if (isActive)
	{
		PurgeComm(Serial, PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
	}

	return isActive;
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
		ClearBuffer();
	}

	LOG_END();
}

void ComPort::TxData(int8_t* data)
{
	assert(data != nullptr && sizeof(data) > 0);
	
	LOG_H("ComPort");

	LOG("--> Send event:\n");
	LOG_T("Data", std::string((char*)data));
	LOG_T("Size", sizeof(data) / sizeof(data[0]));

	if (!WriteFile(Serial, data, sizeof(data) / sizeof(data[0]), NULL, NULL))
	{
		LOG_ERR("Tx Data failed");
	}

	LOG_END();
}

buffer_t* ComPort::RxData()
{
	ClearBuffer();
	buffer.resize(buffer_size);

	if (!ReadFile(Serial, &buffer[0], buffer_size, &readSize, 0))
	{
		LOG_ERR("Rx Data failed");
	}

	if (readSize > 0)
	{
		LOG_H("ComPort");

		LOG("<-- Receive event:\n");
		LOG_TM("Data", std::string((char*)buffer.data()));
		LOG_T("Size", readSize);

		LOG_END();

		buffer = { buffer.begin(), buffer.begin() + readSize };
		return &buffer;
	}

	return nullptr;
}

bool ComPort::IsOpen() const
{
	return isActive;
}

void ComPort::SetBufferSize(uint64_t buffer_size)
{
	this->buffer_size = buffer_size;
	buffer.resize(buffer_size);
}

void ComPort::ClearBuffer()
{
	buffer.clear();
}

std::string ComPort::GetName() const
{
	return name;
}

#undef CHECK_ERROR