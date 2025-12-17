#include "ledeffects.h"

bool LedEffects::initLedc() {
    // Инициализация LEDC
    // Конфигурация таймера
    ledc_timer_config_t timer_conf = {
        .speed_mode = ledcConfig.mode,
        .duty_resolution = ledcConfig.resolution,
        .timer_num = ledcConfig.timer,
        .freq_hz = ledcConfig.frequency,
        .clk_cfg = LEDC_AUTO_CLK};
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        Serial.print("LEDC timer error: ");
        Serial.println(err);
        return false;
    }
    // Конфигурация канала
    ledc_channel_config_t channel_conf = {
        .gpio_num = pin,
        .speed_mode = ledcConfig.mode,
        .channel = ledcConfig.channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = ledcConfig.timer,
        .duty = 0,
        .hpoint = 0};
    err = ledc_channel_config(&channel_conf);
    if (err != ESP_OK) {
        Serial.print("LEDC channel error: ");
        Serial.println(err);
        return false;
    }
    ledcInitialized = true;
    // Привязываем пин к LEDC для JLed
    ledcAttachPin(pin, ledcConfig.channel);
    return true;
}

void LedEffects::effectTaskWrapper(void *parameter) {
    LedEffects *parser = (LedEffects *)parameter;
    parser->effectTask();
}

void LedEffects::effectTask() {
    bool immediate;
    char cmd[MAX_EFFECT_LENGTH];
    while (taskRunning) {
        immediate = false;
        if (!led->IsRunning()) {
            // сначала внутренняя очередь
            if (uxQueueMessagesWaiting(sequenceQueue) > 0) {
                if (xQueueReceive(sequenceQueue, cmd, 0) == pdTRUE) {
                    executeEffect(String(cmd));
                }
            }
            // если она пуста — берём внешнюю
            else if (xQueueReceive(effectQueue, cmd, 0) == pdTRUE) {
                expandSequence(cmd);
                immediate = true;
            }
        }
        led->Update();
        if (!immediate) {
            vTaskDelay(pdMS_TO_TICKS(taskConfig.updateInterval));
        }
    }
    vTaskDelete(NULL);
}

void LedEffects::parseEffectString(String input, String *tokens, int maxTokens, int &count) {
    input.trim();
    count = 0;
    int startPos = 0;
    while (count < maxTokens && startPos < input.length()) {
        // Пропускаем множественные пробелы
        while (startPos < input.length() && input.charAt(startPos) == ' ') {
            startPos++;
        }
        if (startPos >= input.length())
            break;
        int spacePos = input.indexOf(' ', startPos);
        if (spacePos == -1) {
            tokens[count++] = input.substring(startPos);
            break;
        }
        tokens[count++] = input.substring(startPos, spacePos);
        startPos = spacePos + 1;
    }
}

// Парсинг и выполнение одной команды эффекта (без разделителей)
bool LedEffects::executeEffect(String effectString) {
    String tokens[6];
    int tokenCount;
    parseEffectString(effectString, tokens, 6, tokenCount);
    if (tokenCount == 0) {
        return false;
    }
    String effect = tokens[0];
    effect.toLowerCase();
    // blink: "blink <period> [on_time] [duration]"
    if (effect == "blink") {
        if (tokenCount < 2) {
            return false;
        }
        uint32_t period = tokens[1].toInt();
        uint32_t onTime = (tokenCount >= 3) ? tokens[2].toInt() : (period / 2);
        uint32_t totalDuration = (tokenCount >= 4) ? tokens[3].toInt() : 5000;
        uint16_t repeats = totalDuration / period;
        if (repeats == 0) {
            repeats = 1;
        }
        led->Blink(onTime, period - onTime).Repeat(repeats);
        return true;
    }
    // breathe: "breathe <cycle> [duration]"
    else if (effect == "breathe") {
        if (tokenCount < 2) {
            return false;
        }
        uint32_t cycle = tokens[1].toInt();
        uint32_t totalDuration = (tokenCount >= 3) ? tokens[2].toInt() : (cycle * 3);
        uint16_t cycles = totalDuration / cycle;
        if (cycles == 0) {
            cycles = 1;
        }
        led->Breathe(cycle).Repeat(cycles);
        return true;
    }
    // fadein: "fadein <duration>"
    else if (effect == "fadein") {
        if (tokenCount < 2) {
            return false;
        }
        led->FadeOn(tokens[1].toInt());
        return true;
    }
    // fadeout: "fadeout <duration>"
    else if (effect == "fadeout") {
        if (tokenCount < 2) {
            return false;
        }
        led->FadeOff(tokens[1].toInt());
        return true;
    }
    // candle: "candle [period] [speed] [duration]"
    else if (effect == "candle") {
        uint16_t period = (tokenCount >= 2) ? tokens[1].toInt() : 100;
        uint8_t speed = (tokenCount >= 3) ? tokens[2].toInt() : 10;
        uint32_t totalDuration = (tokenCount >= 4) ? tokens[3].toInt() : 5000;
        uint16_t repeats = totalDuration / period;
        if (repeats == 0) {
            repeats = 1;
        }
        led->Candle(period, speed).Repeat(repeats);
        return true;
    }
    // wait: "wait <duration>"
    // Использует эффект дыхания с одним циклом для сохранения времени
    else if (effect == "wait") {
        if (tokenCount < 2) {
            return false;
        }
        uint32_t duration = tokens[1].toInt();
        // Используем один цикл Breathe для создания паузы
        // Breathe плавно меняет яркость, но один длинный цикл
        // создаёт эффект плавного удержания состояния
        led->Breathe(duration).Repeat(1);
        return true;
    }
    // on: "on [brightness] [duration]"
    else if (effect == "on") {
        uint8_t brightness = (tokenCount >= 2) ? tokens[1].toInt() : 255;
        if (tokenCount >= 3) {
            // С длительностью - разделяем на on + wait
            uint32_t duration = tokens[2].toInt();
            // Включаем на нужную яркость
            led->On().Set(brightness);
            // Не используем wait здесь, т.к. это одна команда
            // Вместо этого используем Blink с длинным включением
            led->Blink(duration - 1, 1).Set(brightness).Repeat(1);
            return true;
        } else {
            // Без длительности - постоянное включение
            led->On().Set(brightness);
            return true;
        }
    }
    // off: "off [duration]"
    else if (effect == "off") {
        if (tokenCount >= 2) {
            // С длительностью - используем Blink с длинным выключением
            uint32_t duration = tokens[1].toInt();
            led->Blink(1, duration - 1).Set(0).Repeat(1);
            return true;
        } else {
            // Без длительности - постоянное выключение
            led->Off();
            return true;
        }
    }
    // repeat: "repeat <n> <effect> ..."
    else if (effect == "repeat") {
        if (tokenCount < 3) {
            return false;
        }
        uint16_t count = tokens[1].toInt();
        String subEffect = "";
        for (int i = 2; i < tokenCount; i++) {
            subEffect += tokens[i];
            if (i < tokenCount - 1) {
                subEffect += " ";
            }
        }
        executeEffect(subEffect);
        led->Repeat(count);
        return true;
    }
    return false;
}

void LedEffects::expandSequence(String cmd) {
    char buf[MAX_EFFECT_LENGTH];
    int idx = 0;
    for (int i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == '|') {
            if (idx > 0) {
                buf[idx] = '\0';
                xQueueSend(sequenceQueue, buf, portMAX_DELAY);
                idx = 0;
            }
        } else if (cmd[i] != ' ') {
            buf[idx++] = cmd[i];
        }
    }
    if (idx > 0) {
        buf[idx] = '\0';
        xQueueSend(sequenceQueue, buf, portMAX_DELAY);
    }
}

bool LedEffects::startTask() {
    if (taskRunning) {
        return false;
    }
    // Инициализируем LEDC перед запуском задачи
    if (!ledcInitialized) {
        if (!initLedc()) {
            Serial.println("Ошибка инициализации LEDC");
            return false;
        }
    }
    taskRunning = true;
    BaseType_t result;
    if (taskConfig.coreId == -1) {
        result = xTaskCreate(
            effectTaskWrapper, "LedEffectTask",
            taskConfig.stackSize, this,
            taskConfig.priority, &taskHandle);
    } else {
        result = xTaskCreatePinnedToCore(
            effectTaskWrapper, "LedEffectTask",
            taskConfig.stackSize, this,
            taskConfig.priority, &taskHandle,
            taskConfig.coreId);
    }
    return result == pdPASS;
}

void LedEffects::stopTask() {
    taskRunning = false;
    xQueueReset(effectQueue);
    xQueueReset(sequenceQueue);
    vTaskDelay(pdMS_TO_TICKS(100));
}

bool LedEffects::restartTask() {
    stopTask();
    return startTask();
}

// Настройки LEDC
void LedEffects::setLedcChannel(ledc_channel_t channel) {
    ledcConfig.channel = channel;
}

void LedEffects::setLedcTimer(ledc_timer_t timer) {
    ledcConfig.timer = timer;
}

void LedEffects::setLedcFrequency(uint32_t freq) {
    ledcConfig.frequency = freq;
    if (ledcInitialized) {
        ledc_set_freq(ledcConfig.mode, ledcConfig.timer, freq);
    }
}

void LedEffects::setLedcResolution(ledc_timer_bit_t resolution) {
    ledcConfig.resolution = resolution;
}

// Изменить частоту ШИМ на лету
bool LedEffects::updateLedcFrequency(uint32_t freq) {
    if (!ledcInitialized) {
        return false;
    }
    ledcConfig.frequency = freq;
    return ledc_set_freq(ledcConfig.mode, ledcConfig.timer, freq) == ESP_OK;
}

// Настройки задачи
void LedEffects::setCoreId(BaseType_t core) {
    taskConfig.coreId = core;
}

void LedEffects::setPriority(UBaseType_t priority) {
    taskConfig.priority = priority;
    if (taskRunning && taskHandle != NULL) {
        vTaskPrioritySet(taskHandle, priority);
    }
}

void LedEffects::setUpdateInterval(uint16_t interval) {
    taskConfig.updateInterval = interval;
}

void LedEffects::setStackSize(uint32_t size) {
    taskConfig.stackSize = size;
}

// Установить паузу между эффектами (мс)
void LedEffects::setPauseBetweenEffects(uint32_t pause) {
    taskConfig.pauseBetweenEffects = pause;
}

// Получить текущую паузу
uint32_t LedEffects::getPauseBetweenEffects() const {
    return taskConfig.pauseBetweenEffects;
}

LedEffectsTask LedEffects::getTaskConfig() const {
    return taskConfig;
}

LedEffectsLedc LedEffects::getLedcConfig() const {
    return ledcConfig;
}

void LedEffects::printTaskInfo() {
    // TODO Remove ?
    Serial.println("=== LED Task Info ===");
    Serial.print("Pin: ");
    Serial.println(pin);
    Serial.print("Core: ");
    Serial.println(taskConfig.coreId == -1 ? "Any" : String(taskConfig.coreId));
    Serial.print("Priority: ");
    Serial.println(taskConfig.priority);
    Serial.print("Stack: ");
    Serial.print(taskConfig.stackSize);
    Serial.println(" bytes");
    Serial.print("Update: ");
    Serial.print(taskConfig.updateInterval);
    Serial.println(" ms");
    Serial.print("Pause: ");
    Serial.print(taskConfig.pauseBetweenEffects);
    Serial.println(" ms");
    Serial.print("Effect Queue: ");
    Serial.print(taskConfig.effectQueueSize);
    Serial.print("Sequence Queue: ");
    Serial.print(taskConfig.sequenceQueueSize);
    Serial.print(" (");
    Serial.print(getQueueCount());
    Serial.println(" used)");
    Serial.print("Running: ");
    Serial.println(taskRunning ? "Yes" : "No");

    if (taskRunning && taskHandle != NULL) {
        Serial.print("Free stack: ");
        Serial.print(uxTaskGetStackHighWaterMark(taskHandle));
        Serial.println(" bytes");
    }

    Serial.println("\n=== LEDC Config ===");
    Serial.print("Channel: ");
    Serial.println(ledcConfig.channel);
    Serial.print("Timer: ");
    Serial.println(ledcConfig.timer);
    Serial.print("Frequency: ");
    Serial.print(ledcConfig.frequency);
    Serial.println(" Hz");
    Serial.print("Resolution: ");
    Serial.print(ledcConfig.resolution);
    Serial.println(" bit");
    Serial.print("Mode: ");
    Serial.println(ledcConfig.mode == LEDC_LOW_SPEED_MODE ? "Low Speed" : "High Speed");
    Serial.print("Initialized: ");
    Serial.println(ledcInitialized ? "Yes" : "No");
    Serial.println("====================");
}

bool LedEffects::addEffect(String effect) {
    if (effect.length() >= MAX_EFFECT_LENGTH) {
        return false;
    }
    char effectStr[MAX_EFFECT_LENGTH];
    effect.toCharArray(effectStr, MAX_EFFECT_LENGTH);
    return xQueueSend(effectQueue, effectStr, pdMS_TO_TICKS(100)) == pdTRUE;
}

bool LedEffects::addEffectPriority(String effect) {
    if (effect.length() >= MAX_EFFECT_LENGTH) {
        return false;
    }
    char effectStr[MAX_EFFECT_LENGTH];
    effect.toCharArray(effectStr, MAX_EFFECT_LENGTH);
    return xQueueSendToFront(effectQueue, effectStr, pdMS_TO_TICKS(100)) == pdTRUE;
}

void LedEffects::clearQueue() {
    xQueueReset(effectQueue);
}

int LedEffects::getQueueCount() {
    return uxQueueMessagesWaiting(effectQueue);
}

bool LedEffects::isRunning() {
    return led->IsRunning();
}

void LedEffects::stopLed() {
    led->Stop();
}

void LedEffects::resetLed() {
    led->Reset();
}
