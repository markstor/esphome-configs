#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"

// Libraries for CC1101 module
#include <ELECHOUSE_CC1101_SRC_DRV.h>


namespace esphome {
namespace cc1101 {

static const char *TAG = "cc1101.cc1101";

class CC1101 : public Component {
 public:
  void setup() override;
  // void loop() override;
  void dump_config() override;

  void set_emitter_pin(InternalGPIOPin *emitter) { this->emitter_pin_ = emitter; }
  void set_cc1101_frequency(double frequency) { this->cc1101_frequency_ = frequency; }

  uint8_t get_emitter_pin();

  template<typename Func> void transmit(Func &&action, double frequency = -1) {
    if (frequency > 0) {
    	ESP_LOGI(TAG, "Switching to %0.2fMHz", frequency);
    	ELECHOUSE_cc1101.setMHZ(frequency);
    }
    ELECHOUSE_cc1101.SetTx();
    action();
    ELECHOUSE_cc1101.setSidle();
  }

 protected:
  InternalGPIOPin *emitter_pin_{nullptr};
  double cc1101_frequency_{433.42};
};

}  // namespace cc1101
}  // namespace esphome
