/*
* Исходный файл кода прошивки МК
* Данный файл является предрелизной версией ИЛИ черновиком для
* последних версий.
*/

#include <Arduino.h>

typedef unsigned char      uint8_t;
typedef unsigned long long uint64_t;

#define SPLITTER String(":")
#define OUTPORT(header, data) Serial.println(String(header) + SPLITTER + String(data))

#define _500ms  500   // 500 мс
#define _1000ms 1000  // 1000 мс
#define _2000ms 2000  // 2000 мс
#define _5000ms 5000  // 5000 мс

// Структура потока
struct thread
{
    // period - период выполнения функции action,
    // action - функция для выполнения по таймеру
    thread(uint16_t period = 100, void (*action) () = nullptr)
        :
        period(period),
        action(action)
    {
    }

    // Процесс
    void go()
    {
        if (millis() - timer > period)
        {
            timer = millis();

            // Вызов функции, если она определена
            if (action != nullptr)
            {
                action();
            }
        }
        
    }

    uint16_t period;
    uint64_t timer = 0;
    void     (*action) ();
};

/* БЛОК ПОЛУЧЕНИЯ И ВЫДАЧИ ДАННЫХ */

// Функция выдачи данных о температуре
void EchoTemperature()
{
    OUTPORT("T", "23.0");
}

// Функция выдачи данных о напряжении
void EchoVoltage()
{
    OUTPORT("V", "12.0");
}

// Функция выдачи данных о токе
void EchoCurrent()
{
    OUTPORT("C", "1.0");
}

/**********************************/

/* ДИСПЕТЧЕРЫ ЦИКЛОВ */

// Диспетчер 500мс
void disp_500ms()
{
    EchoTemperature();
}

// Диспетчер 1000мс
void disp_1000ms()
{
    EchoCurrent();
    EchoVoltage();
}

// Диспетчер 5000мс
void disp_5000ms()
{

}

/*********************/

void setup()
{
    Serial.begin(9600);
    Serial.setTimeout(10);

    // Эмуляция потоков данных
    thread workers[3];
    workers[0] = thread(_500ms,  &disp_500ms); 
    workers[1] = thread(_1000ms, &disp_1000ms);
    workers[2] = thread(_5000ms, &disp_5000ms);

    while (!false)
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            workers[i].go();
        }
    }
}

void loop() {}