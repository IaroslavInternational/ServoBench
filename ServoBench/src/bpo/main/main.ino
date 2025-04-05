/*
* Исходный файл кода прошивки МК
* Данный файл является предрелизной версией ИЛИ черновиком для
* последних версий.
*/

#include <Arduino.h>

#include "OneWire.h"
#include "DallasTemperature.h"

bool IsTempOut = false;
OneWire oneWire(2);  
DallasTemperature ds(&oneWire);

typedef unsigned char      uint8_t;
typedef unsigned long long uint64_t;

#define SPLITTER ":"
#define OUTPORT(header, data) Serial.println(String(header) + String(SPLITTER) + String(data))

#define _100ms  100   // 100 мс
#define _500ms  500   // 500 мс
#define _1000ms 1000  // 1000 мс
#define _2000ms 2000  // 2000 мс
#define _5000ms 5000  // 5000 мс

// Структура диспетчера
struct dispatch
{
    // period - период выполнения функции action,
    // action - функция для выполнения по таймеру
    dispatch(uint16_t period = 100, void (*action) () = nullptr)
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

String GetRandom(int min, int max)
{
    return String(random(min, max));
}

/* БЛОК ПОЛУЧЕНИЯ И ВЫДАЧИ ДАННЫХ */

// Функция выдачи данных о температуре
void EchoTemperature()
{
    IsTempOut = true;
    OUTPORT("T", ds.getTempCByIndex(0));
}

// Функция выдачи данных о напряжении
void EchoVoltage()
{
    OUTPORT("V", GetRandom(8, 22));
}

// Функция выдачи данных о токе
void EchoCurrent()
{
    OUTPORT("C", GetRandom(1, 5));
}

/**********************************/

/* ДИСПЕТЧЕРЫ ЦИКЛОВ */

// Диспетчер 100мс
void disp_100ms()
{
    EchoTemperature();
}

// Диспетчер 500мс
void disp_500ms()
{
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
    randomSeed(analogRead(0)); // Debug random seed
    ds.begin();
    ds.setResolution(9);

    // Настройка порта передачи данных
    Serial.begin(38400);
    Serial.setTimeout(10);

    {
        char   test_buffer[64];
        String raw = "";

        while (!false)
        {
            if (Serial.available() > 0)
            {
                memset(test_buffer, '\0', 64);
                Serial.readBytes(test_buffer, 64);

                raw = String(test_buffer);

                if (!strcmp(raw.c_str(), "go"))
                {
                    break;
                }
            }
        }
    }

    // Диспетчеры
    dispatch dsps[3];

    // Эмуляция потоков данных
    dsps[0] = dispatch(_100ms,  &disp_100ms);
    dsps[1] = dispatch(_1000ms, &disp_1000ms);
    dsps[2] = dispatch(_5000ms, &disp_5000ms);

    char   input_buffer[64];
    String raw = "";

    // Основной цикл
    while (!false)
    {
        if (IsTempOut)
        {
            ds.requestTemperatures();
            IsTempOut = false;
        }

        if (Serial.available() > 0)
        {
            memset(input_buffer, '\0', 64);
            Serial.readBytes(input_buffer, 64);

            raw = String(input_buffer);

            if (!strcmp(raw.c_str(), "out"))
            {
                asm volatile("jmp 0x00");
            }
        }

        for (uint8_t i = 0; i < 3; i++)
        {
            dsps[i].go();
        }
    }
}

void loop() {}