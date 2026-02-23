#include "Lilygot547Battery.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lilygo_t5_47_battery {

static const char *TAG = "lilygo_t5_47_battery";

void Lilygot547Battery::setup() {
  // Configure ADC oneshot unit
  adc_oneshot_unit_init_cfg_t unit_cfg = {};
  unit_cfg.unit_id = ADC_UNIT_1;
  esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &this->adc_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(err));
    return;
  }

  // Configure channel (GPIO36 = ADC1_CHANNEL_0)
  adc_oneshot_chan_cfg_t chan_cfg = {};
  chan_cfg.atten = ADC_ATTEN_DB_12;
  chan_cfg.bitwidth = ADC_BITWIDTH_12;
  err = adc_oneshot_config_channel(this->adc_handle, ADC_CHANNEL_0, &chan_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(err));
    return;
  }

  // Set up calibration
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_cfg = {};
  cali_cfg.unit_id = ADC_UNIT_1;
  cali_cfg.atten = ADC_ATTEN_DB_12;
  cali_cfg.bitwidth = ADC_BITWIDTH_12;
  err = adc_cali_create_scheme_curve_fitting(&cali_cfg, &this->cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_cfg = {};
  cali_cfg.unit_id = ADC_UNIT_1;
  cali_cfg.atten = ADC_ATTEN_DB_12;
  cali_cfg.bitwidth = ADC_BITWIDTH_12;
#if CONFIG_IDF_TARGET_ESP32
  cali_cfg.default_vref = 1100;
#endif
  err = adc_cali_create_scheme_line_fitting(&cali_cfg, &this->cali_handle);
#else
  err = ESP_ERR_NOT_SUPPORTED;
#endif
  if (err == ESP_OK) {
    this->calibrated = true;
  } else {
    ESP_LOGW(TAG, "ADC calibration not available: %s", esp_err_to_name(err));
  }
}

void Lilygot547Battery::update() {
  delay(100);
  Lilygot547Battery::update_battery_info();
}

void Lilygot547Battery::update_battery_info() {
  if (this->adc_handle == nullptr) {
    return;
  }

  int raw = 0;
  esp_err_t err = adc_oneshot_read(this->adc_handle, ADC_CHANNEL_0, &raw);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read ADC: %s", esp_err_to_name(err));
    return;
  }

  double battery_voltage;
  if (this->calibrated) {
    int voltage_mv = 0;
    adc_cali_raw_to_voltage(this->cali_handle, raw, &voltage_mv);
    // Multiply by 2 because of the voltage divider on the board
    battery_voltage = (voltage_mv / 1000.0) * 2.0;
  } else {
    // Fallback: manual calculation with default vref of 1100mV
    battery_voltage = ((double) raw / 4095.0) * 2.0 * 3.3 * (1100.0 / 1000.0);
  }

  if (this->voltage != nullptr) {
    this->voltage->publish_state(battery_voltage);
  }
}

}  // namespace lilygo_t5_47_battery
}  // namespace esphome
