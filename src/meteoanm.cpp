#include "meteoanm.h"

bool MCPWMFreqCounter::begin() {
    mcpwm_gpio_init(unit, MCPWM_CAP_0, (gpio_num_t)pin);

    mcpwm_capture_config_t conf = {};
    conf.cap_edge = MCPWM_NEG_EDGE;
    conf.cap_prescale = 1; // тик = 1/80 МГц
    conf.capture_cb = isrHandler;
    conf.user_data = this;

    if (mcpwm_capture_enable_channel(unit, cap, &conf) != ESP_OK)
        return false;

    return true;
}

bool IRAM_ATTR MCPWMFreqCounter::isrHandler(
    mcpwm_unit_t unit,
    mcpwm_capture_channel_id_t cap_channel,
    const cap_event_data_t *edata,
    void *user_data) {
    MCPWMFreqCounter *self = (MCPWMFreqCounter *)user_data;

    uint32_t ticks = edata->cap_value; // аппаратный timestamp
    self->onCapture(ticks);

    return true; // разрешаем обработку
}

void IRAM_ATTR MCPWMFreqCounter::onCapture(uint32_t now) {
    if (debounceTicks && (uint32_t)(now - lastTickTs) < debounceTicks) {
        return;
    }
    lastTickTs = now;
    lastRealUs = esp_timer_get_time();
    pushTimestamp(now);
}

void IRAM_ATTR MCPWMFreqCounter::pushTimestamp(uint32_t t) {
    uint16_t h = head;
    uint16_t nh = (h + 1) % BUF_SIZE;

    buf[h] = t;
    head = nh;

    uint32_t limit = t - (uint32_t)windowTicks;

    while (tail != head) {
        uint32_t ts = buf[tail];
        if ((int32_t)(ts - limit) >= 0)
            break;
        tail = (tail + 1) % BUF_SIZE;
    }
}

uint32_t MCPWMFreqCounter::getCount() {
    int64_t nowUs = esp_timer_get_time();
    int64_t dUs = nowUs - lastRealUs;
    uint32_t deltaTicks = dUs * (APB_CLK_FREQ / 1000000);
    uint32_t virtualNow = lastTickTs + deltaTicks;
    uint32_t limit = virtualNow - windowTicks;
    noInterrupts();
    while (tail != head) {
        uint32_t ts = buf[tail];
        if ((int32_t)(ts - limit) >= 0)
            break;
        tail = (tail + 1) % BUF_SIZE;
    }
    if (tail == head) {
        interrupts();
        return 0;
    }
    uint32_t n = (head - tail + BUF_SIZE) % BUF_SIZE;
    interrupts();
    return n;
}

float MCPWMFreqCounter::getFrequency() {
    uint32_t n = getCount();
    return (float)n * (APB_CLK_FREQ / (float)windowTicks);
}