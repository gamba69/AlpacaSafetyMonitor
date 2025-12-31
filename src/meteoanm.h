#pragma once

#include <Arduino.h>
#include <driver/pcnt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// Структура для передачи данных через очередь
struct PCNTCounterSnapshot {
    uint32_t timestamp;
    int32_t count;
};

class PCNTFrequencyCounter {
  private:
    // Конфигурация
    gpio_num_t inputPin;
    pcnt_unit_t pcntUnit;
    uint32_t samplePeriodMs;
    // Циркулярный буфер
    static const int BUFFER_SIZE = 200;
    PCNTCounterSnapshot buffer[BUFFER_SIZE];
    int bufferIndex;
    int bufferFilled;
    // FreeRTOS объекты
    QueueHandle_t dataQueue;
    TaskHandle_t processingTask;
    hw_timer_t *tmr;
    // Переменные для ISR
    volatile int32_t overflowCount;
    portMUX_TYPE spinlock;
    // Статические указатели для доступа из ISR
    static PCNTFrequencyCounter *instances[PCNT_UNIT_MAX];
    // ISR обработчики
    static void IRAM_ATTR pcntISR(void *arg);
    static void IRAM_ATTR tmrISR();
    // Фоновая задача обработки данных
    static void processingTaskFunction(void *parameter);
    // Настройка
    void initPCNT();
    void initTMR();

  public:
    PCNTFrequencyCounter(gpio_num_t pin, pcnt_unit_t unit = PCNT_UNIT_0, uint32_t sampleMs = 100)
        : inputPin(pin), pcntUnit(unit), samplePeriodMs(sampleMs),
          bufferIndex(0), bufferFilled(0), overflowCount(0),
          dataQueue(NULL), processingTask(NULL), tmr(NULL) {
        spinlock = portMUX_INITIALIZER_UNLOCKED;
    }
    ~PCNTFrequencyCounter() {
        end();
    }
    bool begin(UBaseType_t = 1);
    void end();
    // Получение количества импульсов в скользящем окне
    uint64_t getCount(uint32_t windowSizeMs);
    // Получение частоты в Гц
    double getFrequency(uint32_t windowSizeMs);
    // Получение мгновенного значения счетчика
    uint64_t getCurrentCount();
    // Сброс счетчика
    void resetCounter();
};

