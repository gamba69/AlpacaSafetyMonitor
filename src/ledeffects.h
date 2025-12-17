#pragma once

#include <Arduino.h>
#include <JLed.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

/*
===============================================================================
LED EFFECT COMMAND LANGUAGE
===============================================================================

ВАЖНО:
- Поддерживаются ТОЛЬКО ПЛОСКИЕ последовательности через символ '|'
- Вложенные команды, скобки (), {}, [] — НЕ поддерживаются
- Все команды выполняются строго последовательно
- Все команды добавляются асинхронно в очередь
- Выполнение происходит ТОЛЬКО внутри effectTask()

-------------------------------------------------------------------------------
ОБЩИЙ СИНТАКСИС
-------------------------------------------------------------------------------

<command> ::= <single_command> | <single_command> '|' <command>
Примеры:
    "blink 500"
    "blink 500 | wait 1000 | fadeout 300"

-------------------------------------------------------------------------------
БАЗОВЫЕ КОМАНДЫ
-------------------------------------------------------------------------------

1) blink
---------
blink <period_ms> [on_time_ms] [total_duration_ms]
Описание:
    Мигающий эффект (вкл / выкл)
Параметры:
    period_ms          - полный период мигания в миллисекундах
    on_time_ms         - время включения в миллисекундах
    total_duration_ms  - общая длительность эффекта
Значения по умолчанию:
    on_time_ms         = period_ms / 2
    total_duration_ms  = 5000
Повторы:
    repeats = total_duration_ms / period_ms
    если repeats == 0 → repeats = 1
Примеры:
    blink 500
    blink 1000 200
    blink 300 100 3000

2) breathe
-----------
breathe <cycle_ms> [total_duration_ms]
Описание:
    Плавное увеличение и уменьшение яркости
Параметры:
    cycle_ms           - длительность одного цикла (fade in + fade out)
    total_duration_ms  - общая длительность эффекта
Значения по умолчанию:
    total_duration_ms  = cycle_ms * 3
Повторы:
    cycles = total_duration_ms / cycle_ms
    если cycles == 0 → cycles = 1
Примеры:
    breathe 1000
    breathe 800 5000

3) fadein
----------
fadein <duration_ms>
Описание:
    Плавное включение от 0 до текущей яркости
Параметры:
    duration_ms        - длительность эффекта
Примеры:
    fadein 500
    fadein 2000

4) fadeout
-----------
fadeout <duration_ms>
Описание:
    Плавное выключение до 0
Параметры:
    duration_ms        - длительность эффекта
Примеры:
    fadeout 300
    fadeout 1500

5) candle
----------
candle [period_ms] [speed] [total_duration_ms]
Описание:
    Эффект мерцания свечи
Параметры:
    period_ms          - период обновления
    speed              - скорость хаотических изменений
    total_duration_ms  - общая длительность эффекта
Значения по умолчанию:
    period_ms          = 100
    speed              = 10
    total_duration_ms  = 5000
Повторы:
    repeats = total_duration_ms / period_ms
    если repeats == 0 → repeats = 1
Примеры:
    candle
    candle 50
    candle 100 15
    candle 80 12 3000

6) wait
--------
wait <duration_ms>
Описание:
    Пауза без изменения состояния LED
Параметры:
    duration_ms        - длительность паузы
Реализация:
    Используется один длинный цикл breathe
Примеры:
    wait 500
    wait 2000

7) on
------
on [brightness] [duration_ms]
Описание:
    Включить LED
Параметры:
    brightness         - яркость (0..255)
    duration_ms        - длительность удержания
Значения по умолчанию:
    brightness         = 255
    duration_ms        = бесконечно (пока не придёт следующая команда)
Примеры:
    on
    on 128
    on 200 3000

8) off
-------
off [duration_ms]
Описание:
    Выключить LED
Параметры:
    duration_ms        - длительность удержания
Значения по умолчанию:
    duration_ms        = бесконечно (пока не придёт следующая команда)
Примеры:
    off
    off 2000

-------------------------------------------------------------------------------
УПРАВЛЯЮЩИЕ КОМАНДЫ
-------------------------------------------------------------------------------

9) repeat
---------
repeat <count> <single_command>
Описание:
    Повторяет одну команду указанное количество раз
Параметры:
    count              - количество повторов
    single_command     - ОДНА команда (без '|')
Ограничения:
    - Последовательности и скобки НЕ поддерживаются
    - repeat blink 200 | wait 300   ❌ НЕКОРРЕКТНО
Примеры:
    repeat 5 blink 200
    repeat 3 breathe 1000

-------------------------------------------------------------------------------
ПОСЛЕДОВАТЕЛЬНОСТИ
-------------------------------------------------------------------------------

A | B | C
Описание:
    Последовательное выполнение команд
Правила:
    - Команды выполняются строго слева направо
    - Следующая команда запускается ТОЛЬКО после завершения предыдущей
    - Допускаются только плоские последовательности
Примеры:
    blink 300 | wait 500 | fadeout 200
    on 128 | wait 1000 | off

-------------------------------------------------------------------------------
ПРИМЕРЫ КОРРЕКТНЫХ КОМАНД
-------------------------------------------------------------------------------

"blink 500"
"blink 300 | wait 500 | fadeout 200"
"repeat 5 blink 200"
"on 128 | wait 1000 | off"

-------------------------------------------------------------------------------
ПРИМЕРЫ НЕКОРРЕКТНЫХ КОМАНД
-------------------------------------------------------------------------------

"(blink 200 | wait 300)"        // скобки не поддерживаются
"repeat 3 blink 200 | wait 300" // repeat только для одной команды
"A || B"                        // двойной разделитель
"| blink 200"                   // пустая команда

===============================================================================
*/

// Настройки LEDC для ESP32-S3
struct LedEffectsLedc {
    ledc_channel_t channel;      // LEDC канал (0-7 для ESP32-S3)
    ledc_timer_t timer;          // LEDC таймер (0-3)
    ledc_mode_t mode;            // Режим: LEDC_LOW_SPEED_MODE
    uint32_t frequency;          // Частота ШИМ (Гц)
    ledc_timer_bit_t resolution; // Разрешение (биты)

    LedEffectsLedc() {
        channel = LEDC_CHANNEL_0;
        timer = LEDC_TIMER_0;
        mode = LEDC_LOW_SPEED_MODE;
        frequency = 5000;              // 5 кГц (оптимально для LED)
        resolution = LEDC_TIMER_8_BIT; // 8 бит (0-255)
    }
};

// Насройки FreeRTOS задачи
struct LedEffectsTask {
    uint32_t stackSize;
    UBaseType_t priority;
    BaseType_t coreId;
    uint16_t updateInterval;
    uint8_t effectQueueSize;
    uint8_t sequenceQueueSize;
    uint32_t pauseBetweenEffects; // Пауза между эффектами (мс)

    LedEffectsTask() {
        stackSize = 4096;
        priority = 1;
        coreId = 1;          // Core 1 (WiFi на Core 0)
        updateInterval = 10; // 10мс = 100 FPS
        effectQueueSize = 10;
        sequenceQueueSize = 10;
        pauseBetweenEffects = 0; // Без паузы по умолчанию
    }
};

class LedEffects {
  private:
    JLed *led;
    uint8_t pin;
    QueueHandle_t effectQueue;
    QueueHandle_t sequenceQueue;
    TaskHandle_t taskHandle;
    bool taskRunning;
    LedEffectsTask taskConfig;
    LedEffectsLedc ledcConfig;
    bool ledcInitialized;
    static const int MAX_EFFECT_LENGTH = 80; // Увеличено для длинных команд

    bool initLedc();
    static void effectTaskWrapper(void *);
    void effectTask();
    void parseEffectString(String, String *, int, int &);
    bool executeEffect(String);
    void expandSequence(String);

  public:
    LedEffects(uint8_t ledPin)
        : pin(ledPin), taskRunning(false), ledcInitialized(false) {
        led = new JLed(pin);
        effectQueue = xQueueCreate(taskConfig.effectQueueSize, MAX_EFFECT_LENGTH);
        sequenceQueue = xQueueCreate(taskConfig.sequenceQueueSize, MAX_EFFECT_LENGTH);
    }

    LedEffects(uint8_t ledPin, const LedEffectsTask &tConfig, const LedEffectsLedc &lConfig)
        : pin(ledPin), taskRunning(false), taskConfig(tConfig), ledcConfig(lConfig), ledcInitialized(false) {
        led = new JLed(pin);
        effectQueue = xQueueCreate(taskConfig.effectQueueSize, MAX_EFFECT_LENGTH);
        sequenceQueue = xQueueCreate(taskConfig.sequenceQueueSize, MAX_EFFECT_LENGTH);
    }

    ~LedEffects() {
        stopTask();
        if (ledcInitialized) {
            ledc_stop(ledcConfig.mode, ledcConfig.channel, 0);
        }
        vQueueDelete(effectQueue);
        vQueueDelete(sequenceQueue);
        delete led;
    }

    bool startTask();
    void stopTask();
    bool restartTask();
    // Настройки LEDC
    void setLedcChannel(ledc_channel_t);
    void setLedcTimer(ledc_timer_t);
    void setLedcFrequency(uint32_t);
    void setLedcResolution(ledc_timer_bit_t);
    // Изменить частоту ШИМ на лету
    bool updateLedcFrequency(uint32_t);
    // Настройки задачи
    void setCoreId(BaseType_t);
    void setPriority(UBaseType_t);
    void setUpdateInterval(uint16_t);
    void setStackSize(uint32_t);
    // Установить паузу между эффектами (мс)
    void setPauseBetweenEffects(uint32_t);

    // Получить текущую паузу
    uint32_t getPauseBetweenEffects() const;

    LedEffectsTask getTaskConfig() const;
    LedEffectsLedc getLedcConfig() const;

    void printTaskInfo();

    bool addEffect(String);

    bool addEffectPriority(String);

    void clearQueue();
    int getQueueCount();
    bool isRunning();
    void stopLed();
    void resetLed();
};

// LedEffects *ledParser;

// void setup() {
//     Serial.begin(115200);
//     delay(1000);

//     // Настройки для ESP32-S3
//     LedEffectsTask taskCfg;
//     taskCfg.stackSize = 8192;
//     taskCfg.priority = 2;
//     taskCfg.coreId = 1;         // Core 1 для LED
//     taskCfg.updateInterval = 5; // 200 FPS
//     taskCfg.queueSize = 20;
//     taskCfg.pauseBetweenEffects = 500; // 500мс пауза между эффектами

//     // Настройки LEDC
//     LedEffectsLedc ledcCfg;
//     ledcCfg.channel = LEDC_CHANNEL_0;
//     ledcCfg.timer = LEDC_TIMER_0;
//     ledcCfg.mode = LEDC_LOW_SPEED_MODE;
//     ledcCfg.frequency = 5000;               // 5 кГц (стандарт для LED)
//     ledcCfg.resolution = LEDC_TIMER_13_BIT; // 13 бит для плавности

//     ledParser = new LedEffects(2, taskCfg, ledcCfg);

//     if (ledParser->startTask()) {
//         Serial.println("Задача запущена на ESP32-S3 с LEDC");
//         ledParser->printTaskInfo();
//     } else {
//         Serial.println("Ошибка запуска");
//     }

//     ledParser->addEffect("fadein 1000");
//     ledParser->addEffect("breathe 2000 10000"); // Дыхание 2 сек, всего 10 сек (5 циклов)
//     ledParser->addEffect("blink 300 150 3000"); // Мигание 300мс, всего 3 секунды
//     ledParser->addEffect("fadeout 1000");
//     // Эффекты будут выполняться с паузой 500мс между ними
// }

// void loop() {
//     if (Serial.available() > 0) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
//         if (cmd.length() == 0)
//             return;

//         if (cmd == "info") {
//             ledParser->printTaskInfo();
//         } else if (cmd == "clear") {
//             ledParser->clearQueue();
//             Serial.println("Очередь очищена");
//         } else if (cmd == "stop") {
//             ledParser->stop();
//         } else if (cmd == "count") {
//             Serial.print("В очереди: ");
//             Serial.println(ledParser->getQueueCount());
//         } else if (cmd.startsWith("core ")) {
//             int core = cmd.substring(5).toInt();
//             ledParser->setCoreId(core);
//             ledParser->restartTask();
//             Serial.print("Core: ");
//             Serial.println(core);
//         } else if (cmd.startsWith("pri ")) {
//             int pri = cmd.substring(4).toInt();
//             ledParser->setPriority(pri);
//             Serial.print("Priority: ");
//             Serial.println(pri);
//         } else if (cmd.startsWith("fps ")) {
//             int fps = cmd.substring(4).toInt();
//             ledParser->setUpdateInterval(1000 / fps);
//             Serial.print("FPS: ");
//             Serial.println(fps);
//         } else if (cmd.startsWith("freq ")) {
//             int freq = cmd.substring(5).toInt();
//             if (ledParser->updateLedcFrequency(freq)) {
//                 Serial.print("LEDC freq: ");
//                 Serial.print(freq);
//                 Serial.println(" Hz");
//             }
//         } else if (cmd.startsWith("pause ")) {
//             int pause = cmd.substring(6).toInt();
//             ledParser->setPauseBetweenEffects(pause);
//             Serial.print("Pause: ");
//             Serial.print(pause);
//             Serial.println(" ms");
//         } else if (cmd.startsWith("p ")) {
//             ledParser->addEffectPriority(cmd.substring(2));
//             Serial.println("Приоритетный эффект добавлен");
//         } else {
//             // Обычная команда эффекта
//             if (ledParser->addEffect(cmd)) {
//                 Serial.print("Добавлено: '");
//                 Serial.print(cmd);
//                 Serial.print("' (");
//                 Serial.print(ledParser->getQueueCount());
//                 Serial.println(" в очереди)");
//             } else {
//                 Serial.println("Ошибка: очередь заполнена или команда слишком длинная");
//             }
//         }
//     }

//     vTaskDelay(pdMS_TO_TICKS(100));
// }