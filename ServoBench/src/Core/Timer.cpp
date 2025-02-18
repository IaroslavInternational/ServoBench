#include "Timer.hpp"

void Timer::NewFrame(float dt)
{
	sum += dt;

	if (stamps.size() > limit)
	{
		stamps.erase(stamps.begin());
	}

	stamps.emplace_back(sum);

	// Цикл по всем событиям по таймеру
	if (!eventlist.empty())
	{
		for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
		{
			// Если прошёл период по таймеру
			if (sum / ((float)it->first + it->first * it->second.second) >= 1.0f)
			{
				it->second.second++; // увеличть счётчик вызовов
				it->second.first();  // вызвать ф-ию
			}
		}
	}
}

void Timer::AddAction(uint16_t period, std::function<void()> e)
{
	eventlist.emplace(period, std::pair{ e, 0 });
}

void Timer::Reset()
{
	sum = 0.0f;
	stamps.clear();
	eventlist.clear();
}