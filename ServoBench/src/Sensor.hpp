#pragma once

#include "Core/data_types.hpp"

#include <string>

template<typename T>
class Sensor
{
public:
	Sensor(const std::string& name, const std::string& mask, uint32_t data_limit = 50);
public:
	void Add(T val);
	bool Add(std::string& cmd);
	T*   Get();
	T    GetLast() const;
	std::string&    GetName();
	uint32_t		GetLimit();
	uint64_t		GetSize();
	void		    SetLimit(uint32_t data_limit);
	bool IsEmpty() const;
private:
	std::string    name;
	std::string    mask;
	uint32_t       data_limit;
	std::vector<T> data;
};

template<typename T>
Sensor<T>::Sensor(const std::string& name, const std::string& mask, uint32_t data_limit)
	:
	name(name),
	mask(mask),
	data_limit(data_limit)
{
	data.reserve(data_limit);
}

template<typename T>
void Sensor<T>::Add(T val)
{
	if (data.size() > data_limit)
	{
		data.erase(data.begin() + 1);
	}

	data.emplace_back(val);
}

template<typename T>
bool Sensor<T>::Add(std::string& cmd)
{
	if (cmd.find(mask + ":") != -1 && cmd.size() >= 3)
	{
		cmd = std::string(cmd.begin() + mask.size() + 1, cmd.end());
		
		try
		{
			Add(std::stoi(cmd));
		}
		catch (std::exception& ex)
		{
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

template<typename T>
T* Sensor<T>::Get()
{
	return data.data();
}

template<typename T>
T Sensor<T>::GetLast() const
{
	return data.back();
}

template<typename T>
std::string& Sensor<T>::GetName()
{
	return name;
}

template<typename T>
uint32_t Sensor<T>::GetLimit()
{
	return data_limit;
}

template<typename T>
uint64_t Sensor<T>::GetSize()
{
	return data.size();
}

template<typename T>
void Sensor<T>::SetLimit(uint32_t data_limit)
{
	this->data_limit = data_limit;
	data.reserve(data_limit);
}

template<typename T>
bool Sensor<T>::IsEmpty() const
{
	return data.empty();
}
