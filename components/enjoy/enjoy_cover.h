#pragma once

#include "esphome/components/cover/cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"
#include "esphome/core/component.h"
#include <EnjoyRemote.h>

namespace esphome {
namespace enjoy {

using namespace esphome::cover;

static const char *const TAG = "enjoy.cover";

class EnjoyCover : public Cover, public Component {
protected:
  double freq_;
  cc1101::CC1101 *cc1101_module_;
  EnjoyRemote *remote_;
  const char *storage_key_;
  uint32_t remote_address_;
  int blind_number_;
  bool multicast_;
  int repeat_;

public:
  void setup() override {
    remote_ = new EnjoyRemote(cc1101_module_->get_emitter_pin(), remote_address_, storage_key_);
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void sendCC1101Command(EnjoyCommand command) {
    ESP_LOGI(TAG, "Sending command %01x to blind %d. Counter: %d. Multicast: %d. Repeat: %d", command, this->freq_, this->blind_number_, this->remote_->getCurrentCode(), this->multicast_, this->repeat_);
    this->cc1101_module_->transmit([this, command] { this->remote_->sendCommand(command, this->blind_number_, this->repeat_, this->multicast_); }, this->freq_);
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI(TAG, "OPEN");
        sendCC1101Command(EnjoyCommand::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI(TAG, "CLOSE");
        sendCC1101Command(EnjoyCommand::Down);
      } else {
        ESP_LOGI(TAG, "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI(TAG, "STOP");
      sendCC1101Command(EnjoyCommand::Stop);
    }
  }

  void program() {
    ESP_LOGI(TAG, "PROG");
    sendCC1101Command(EnjoyCommand::Prog);
  }

  void set_remote_address(uint32_t remote_address) { this->remote_address_ = remote_address; }

  void set_storage_key(const char *storage_key) {
    this->storage_key_ = storage_key;
  }
  void set_blind_number(int blindNumber) { this->blind_number_ = blindNumber; }
  void set_repeat(int repeat) { this->repeat_ = repeat; }
  void set_multicast(bool multicast) { this->multicast_ = multicast; }
  void set_frequency(double freq) { this->freq_ = freq; }
  void set_cc1101_module(cc1101::CC1101 *cc1101_module) { this->cc1101_module_ = cc1101_module; }
  void register_program_button(button::Button *button) {
	  button->add_on_press_callback([this] { return this->program(); });
	  
  }
};

} // namespace enjoy
} // namespace esphome
