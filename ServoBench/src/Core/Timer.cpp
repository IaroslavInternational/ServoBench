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

void Timer::AddAction(const log_t& log_data, std::function<void()> e)
{
	eventlist.emplace(log_data.period / 1000.0f, std::pair{ e, uint32_t(sum) });
}

void Timer::Reset()
{
	sum = 0.0f;
	stamps.clear();

	IsTerminate = true;

	// Каждые 50мс ожидаем завершения потоков и закрываем поток
	while (!lib::IsThreadTerminated(thread))
	{
		std::this_thread::sleep_for(50ms);
	}

	eventlist.clear();
}

void Timer::StartEvents()
{
	IsTerminate = false;
	
	for (auto& [period, event_data] : eventlist)
	{
		event_data.second = uint32_t(sum);
	}

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

		// Цикл по всем событиям по таймеру
		if (!eventlist.empty())
		{
			for (auto& [period, event_data] : eventlist)
			{
				// Если прошёл период по таймеру
				if (sum / (period + period * event_data.second) >= 1.0f)
				{
					event_data.second++; // увеличть счётчик вызовов
					event_data.first();  // вызвать ф-ию
				}
			}
		}
	}
}