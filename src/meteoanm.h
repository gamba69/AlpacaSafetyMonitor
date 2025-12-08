#pragma once
#include "driver/mcpwm.h"
#include <Arduino.h>

class MCPWMFreqCounter {
  public:
    MCPWMFreqCounter(int pin, uint32_t windowMs = 3000, uint32_t debounceUs = 0)
        : pin(pin),
          windowTicks((uint64_t)windowMs * 1000 * (APB_CLK_FREQ / 1000000ULL)),
          debounceTicks((uint64_t)debounceUs * (APB_CLK_FREQ / 1000000ULL)) {}

    bool begin();
    float getFrequency();
    uint32_t getCount();

  private:
    static bool IRAM_ATTR isrHandler(
        mcpwm_unit_t unit,
        mcpwm_capture_channel_id_t cap_channel,
        const cap_event_data_t *edata,
        void *user_data);

    void IRAM_ATTR onCapture(uint32_t ticks);
    void IRAM_ATTR pushTimestamp(uint32_t t);

  private:
    int pin;
    const mcpwm_unit_t unit = MCPWM_UNIT_0;
    const mcpwm_capture_channel_id_t cap = MCPWM_SELECT_CAP0;

    static const int BUF_SIZE = 4096;
    volatile uint32_t buf[BUF_SIZE];
    volatile uint16_t head = 0;
    volatile uint16_t tail = 0;

    volatile uint64_t windowTicks;
    volatile uint64_t debounceTicks;

    volatile uint32_t lastTickTs = 0;
    volatile int64_t lastRealUs = 0;
};