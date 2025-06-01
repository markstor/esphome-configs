#include "esphome/core/log.h"
#include "cc1101.h"

namespace esphome {
namespace cc1101 {


void CC1101::setup() {
  // Set EMITTER_GPIO as OUTPUT, otherwise no commands will be sent
  this->emitter_pin_->pin_mode(gpio::FLAG_OUTPUT);
  this->emitter_pin_->digital_write(LOW);

  // Setup SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN
  this->clk_pin_->pin_mode(gpio::FLAG_OUTPUT);
  this->miso_pin_->pin_mode(gpio::FLAG_INPUT);
  this->mosi_pin_->pin_mode(gpio::FLAG_OUTPUT);
  this->cs_pin_->pin_mode(gpio::FLAG_OUTPUT);

  // Set custom spi pins (byte sck, byte miso, byte mosi, byte ss)
  ELECHOUSE_cc1101.setSpiPin(this->clk_pin_->get_pin(), this->miso_pin_->get_pin(), this->mosi_pin_->get_pin(),
                             this->cs_pin_->get_pin());

  ELECHOUSE_cc1101.Init();

  ELECHOUSE_cc1101.setMHZ(this->cc1101_frequency_);

  if (ELECHOUSE_cc1101.getCC1101()) {  // Check the CC1101 Spi connection.
    ESP_LOGI("cc1101", "Connection OK");
  } else {
    ESP_LOGE("cc1101", "Connection Error");
  }
}

void CC1101::dump_config() { ESP_LOGCONFIG(TAG, "CC1101 Module"); }

uint8_t CC1101::get_emitter_pin() { return this->emitter_pin_->get_pin(); }

}  // namespace cc1101
}  // namespace esphome
