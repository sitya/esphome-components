#pragma once
#include <Arduino.h>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

namespace esphome {
namespace lilygo_t5_47_battery {

class Lilygot547Battery : public PollingComponent {
 public:
  sensor::Sensor *voltage{nullptr};

  adc_oneshot_unit_handle_t adc_handle{nullptr};
  adc_cali_handle_t cali_handle{nullptr};
  bool calibrated{false};

  void setup() override;
  void update() override;
  void update_battery_info();

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage = voltage_sensor; }
};
}  // namespace lilygo_t5_47_battery
}  // namespace esphome
