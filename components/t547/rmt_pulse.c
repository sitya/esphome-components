#include "rmt_pulse.h"

#include <driver/rmt.h>
#include <esp_idf_version.h>
#include <rom/ets_sys.h>
#if ESP_IDF_VERSION_MAJOR >= 4
#include <hal/rmt_ll.h>
#endif

static intr_handle_t gRMT_intr_handle = NULL;

// the RMT channel configuration object
static rmt_config_t row_rmt_config;

// keep track of wether the current pulse is ongoing
volatile bool rmt_tx_done = true;

/**
 * Remote peripheral interrupt. Used to signal when transmission is done.
 */
static void IRAM_ATTR rmt_interrupt_handler(void *arg)
{
    rmt_tx_done = true;
    RMT.int_clr.val = RMT.int_st.val;
}

void rmt_pulse_init(gpio_num_t pin)
{
    // FIXME: Temporarily disable RMT initialization to prevent crashes
    // Just set the pin as output for now
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    rmt_tx_done = true;
}

void IRAM_ATTR pulse_ckv_ticks(uint16_t high_time_ticks,
                               uint16_t low_time_ticks, bool wait)
{
    // FIXME: Temporarily disabled RMT to prevent crashes
    // Use simple delay instead (convert ticks to microseconds, assuming 10 ticks = 1us)
    uint32_t delay_us = (high_time_ticks + low_time_ticks) / 10;
    if (delay_us > 0) {
        ets_delay_us(delay_us);
    }
    rmt_tx_done = true;
}

void IRAM_ATTR pulse_ckv_us(uint16_t high_time_us, uint16_t low_time_us,
                            bool wait)
{
    pulse_ckv_ticks(10 * high_time_us, 10 * low_time_us, wait);
}

bool IRAM_ATTR rmt_busy()
{
    return !rmt_tx_done;
}
