#pragma once

#include <future>

namespace lib
{
	// ���������� ��������� ������
	// True  - ����� ���
	// False - ����� �� ����������
	template<typename T>
	bool IsThreadTerminated(const std::future<T>& t)
	{
		return t.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}
}