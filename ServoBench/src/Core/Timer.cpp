#include "Timer.hpp"
#include "lib.hpp"

using namespace std::chrono_literals;

void Timer::NewFrame(float dt)
{
	sum += dt;

	if (stamps.size() > limit)
	{
		stamps.erase(stamps.begin());
	}

	stamps.emplace_back(sum);
}

void Timer::AddAction(uint16_t period, std::function<void()> e)
{
	eventlist.emplace(period, std::pair{ e, uint32_t(sum) });
}

void Timer::Reset()
{
	sum = 0.0f;
	stamps.clear();

	IsTerminate = true;

	// ������ 50�� ������� ���������� ������� � ��������� �����
	while (!lib::IsThreadTerminated(thread))
	{
		std::this_thread::sleep_for(50ms);
	}

	eventlist.clear();
}

void Timer::StartEvents()
{
	IsTerminate = false;
	thread = std::async(std::launch::async, &Timer::ProcEvent, this);
}

void Timer::ProcEvent()
{
	while (!false)
	{
		if (IsTerminate)
		{
			return;
		}

		// ���� �� ���� �������� �� �������
		if (!eventlist.empty())
		{
			for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
			{
				// ���� ������ ������ �� �������
				if (sum / ((float)it->first + it->first * it->second.second) >= 1.0f)
				{
					it->second.second++; // �������� ������� �������
					it->second.first();  // ������� �-��
				}
			}
		}
	}
}