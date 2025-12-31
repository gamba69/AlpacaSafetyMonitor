#include "meteoanm.h"

void IRAM_ATTR PCNTFrequencyCounter::pcntISR(void *arg) {
    PCNTFrequencyCounter *self = static_cast<PCNTFrequencyCounter *>(arg);
    // Получаем статус прерывания
    uint32_t status = 0;
    pcnt_get_event_status(self->pcntUnit, &status);
    // Минимальная обработка в ISR - только учет переполнений
    if (status & PCNT_EVT_H_LIM) {
        self->overflowCount++;
    }
    if (status & PCNT_EVT_L_LIM) {
        self->overflowCount--;
    }
}

void IRAM_ATTR PCNTFrequencyCounter::tmrISR() {
    // Находим нужный экземпляр через статический массив
    for (int i = 0; i < PCNT_UNIT_MAX; i++) {
        PCNTFrequencyCounter *self = instances[i];
        if (self && self->tmr) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            // Быстрое чтение счетчика
            int16_t count;
            pcnt_get_counter_value(self->pcntUnit, &count);
            // Формируем снимок с точной меткой времени
            PCNTCounterSnapshot snapshot;
            snapshot.timestamp = esp_timer_get_time(); // Микросекунды с загрузки
            snapshot.count = (uint64_t)self->overflowCount * 32768ULL + (uint64_t)count;
            // Отправляем в очередь (неблокирующая операция)
            xQueueSendFromISR(self->dataQueue, &snapshot, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
            break;
        }
    }
}

// Фоновая задача обработки данных
void PCNTFrequencyCounter::processingTaskFunction(void *parameter) {
    PCNTFrequencyCounter *self = static_cast<PCNTFrequencyCounter *>(parameter);
    PCNTCounterSnapshot snapshot;
    while (true) {
        // Ожидаем данные из очереди
        if (xQueueReceive(self->dataQueue, &snapshot, portMAX_DELAY) == pdTRUE) {
            // Обработка данных в фоне
            portENTER_CRITICAL(&self->spinlock);
            self->buffer[self->bufferIndex] = snapshot;
            self->bufferIndex = (self->bufferIndex + 1) % BUFFER_SIZE;
            if (self->bufferFilled < BUFFER_SIZE) {
                self->bufferFilled++;
            }
            portEXIT_CRITICAL(&self->spinlock);
        }
    }
}

void PCNTFrequencyCounter::initPCNT() {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = inputPin,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_DIS,
        .neg_mode = PCNT_COUNT_INC,
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,
        .unit = pcntUnit,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config);
    // Фильтр для подавления помех
    pcnt_set_filter_value(pcntUnit, 100);
    pcnt_filter_enable(pcntUnit);
    // События переполнения
    pcnt_event_enable(pcntUnit, PCNT_EVT_H_LIM);
    pcnt_event_enable(pcntUnit, PCNT_EVT_L_LIM);
    // Регистрация ISR
    instances[pcntUnit] = this;
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(pcntUnit, pcntISR, this);
    // Запуск счетчика
    pcnt_counter_pause(pcntUnit);
    pcnt_counter_clear(pcntUnit);
    pcnt_counter_resume(pcntUnit);
}

void PCNTFrequencyCounter::initTMR() {
    tmr = timerBegin(pcntUnit, 80, true);
    timerAttachInterrupt(tmr, &tmrISR, true);
    timerAlarmWrite(tmr, samplePeriodMs * 1000, true);
    timerAlarmEnable(tmr);
}

bool PCNTFrequencyCounter::begin(UBaseType_t taskPriority) {
    // Создаем очередь
    dataQueue = xQueueCreate(16, sizeof(PCNTCounterSnapshot));
    if (dataQueue == NULL) {
        return false;
    }
    // Создаем фоновую задачу
    BaseType_t result = xTaskCreate(
        processingTaskFunction,
        "PCNTFreqCounter",
        4096,
        this,
        taskPriority,
        &processingTask);
    if (result != pdPASS) {
        vQueueDelete(dataQueue);
        dataQueue = NULL;
        return false;
    }
    // Инициализация GPIO
    pinMode(inputPin, INPUT_PULLUP);
    // Инициализация PCNT и таймера
    initPCNT();
    initTMR();
    return true;
}

void PCNTFrequencyCounter::end() {
    if (tmr) {
        timerAlarmDisable(tmr);
        timerDetachInterrupt(tmr);
        timerEnd(tmr);
        tmr = NULL;
    }
    if (processingTask) {
        vTaskDelete(processingTask);
        processingTask = NULL;
    }
    if (dataQueue) {
        vQueueDelete(dataQueue);
        dataQueue = NULL;
    }
    pcnt_isr_handler_remove(pcntUnit);
    pcnt_isr_service_uninstall();
    instances[pcntUnit] = NULL;
}

// Получение количества импульсов в скользящем окне
uint64_t PCNTFrequencyCounter::getCount(uint32_t windowSizeMs) {
    portENTER_CRITICAL(&spinlock);
    if (bufferFilled < 2) {
        portEXIT_CRITICAL(&spinlock);
        return 0; // Недостаточно данных для расчета
    }
    uint64_t currentTime = esp_timer_get_time(); // Микросекунды
    uint64_t windowStart = currentTime - (windowSizeMs * 1000ULL);
    // Последнее значение
    int lastIdx = (bufferIndex - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    uint64_t lastCount = buffer[lastIdx].count;
    uint64_t lastTime = buffer[lastIdx].timestamp;
    // Самое старое доступное значение
    int oldestIdx = (lastIdx - bufferFilled + 1 + BUFFER_SIZE) % BUFFER_SIZE;
    uint64_t oldestTime = buffer[oldestIdx].timestamp;
    // Если запрошенное окно больше доступных данных - используем всё что есть
    if (windowStart < oldestTime) {
        windowStart = oldestTime;
    }
    // Поиск начала окна
    uint64_t startCount = lastCount;
    bool found = false;
    for (int i = 0; i < bufferFilled; i++) {
        int idx = (lastIdx - i + BUFFER_SIZE) % BUFFER_SIZE;
        if (buffer[idx].timestamp <= windowStart) {
            if (i > 0) {
                // Интерполяция между двумя точками
                int prevIdx = (idx + 1) % BUFFER_SIZE;
                uint64_t t1 = buffer[idx].timestamp;
                uint64_t t2 = buffer[prevIdx].timestamp;
                uint64_t c1 = buffer[idx].count;
                uint64_t c2 = buffer[prevIdx].count;
                if (t2 != t1) {
                    double ratio = (double)(windowStart - t1) / (double)(t2 - t1);
                    startCount = c1 + (uint64_t)((int64_t)(c2 - c1) * ratio);
                } else {
                    startCount = c1;
                }
            } else {
                startCount = buffer[idx].count;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        // Используем самое старое значение
        startCount = buffer[oldestIdx].count;
    }
    uint64_t result = lastCount - startCount;
    portEXIT_CRITICAL(&spinlock);
    return result;
}

// Получение частоты в Гц
double PCNTFrequencyCounter::getFrequency(uint32_t windowSizeMs) {
    uint64_t count = getCount(windowSizeMs);
    return (double)count * 1000.0 / windowSizeMs;
}

// Получение мгновенного значения счетчика
uint64_t PCNTFrequencyCounter::getCurrentCount() {
    int16_t count;
    pcnt_get_counter_value(pcntUnit, &count);
    return (uint64_t)overflowCount * 32768ULL + (uint64_t)count;
}

// Сброс счетчика
void PCNTFrequencyCounter::resetCounter() {
    portENTER_CRITICAL(&spinlock);
    pcnt_counter_pause(pcntUnit);
    pcnt_counter_clear(pcntUnit);
    overflowCount = 0;
    bufferIndex = 0;
    bufferFilled = 0;
    pcnt_counter_resume(pcntUnit);
    portEXIT_CRITICAL(&spinlock);
}

// Статический массив экземпляров
PCNTFrequencyCounter *PCNTFrequencyCounter::instances[PCNT_UNIT_MAX] = {nullptr};