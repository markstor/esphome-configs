#pragma once

#include "esphome/components/cover/cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"
#include "esphome/core/component.h"
#include <NVSRollingCodeStorage.h>
#include <SomfyRemote.h>


namespace esphome {
namespace somfy {

using namespace esphome::cover;

static const char *const TAG = "somfy.cover";

class SomfyCover : public Cover, public Component {
protected:
  double freq_;
  cc1101::CC1101 cc1101_module_;
  SomfyRemote *remote_;
  NVSRollingCodeStorage *storage_;
  const char *storage_namespace_;
  const char *storage_key_;
  uint32_t remote_address_;
  int repeat_;

public:
  void setup() override {
    storage_ = new NVSRollingCodeStorage(storage_namespace_, storage_key_);
    remote_ = new SomfyRemote(cc1101_module_->get_emitter_pin(), remote_address_, storage_);
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void sendCC1101Command(Command command) {
	ESP_LOGI(TAG, "Sending command %01x", command);
	this->cc1101_module_->transmit([=] { this->remote_->sendCommand(command, this->repeat_); }, this->freq_);
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI(TAG, "OPEN");
        sendCC1101Command(Command::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI(TAG, "CLOSE");
        sendCC1101Command(Command::Down);
      } else {
        ESP_LOGI(TAG, "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI(TAG, "STOP");
      sendCC1101Command(Command::My);
    }
  }

  void program() {
    ESP_LOGI(TAG, "PROG");
    sendCC1101Command(Command::Prog);
  }
  void set_remote_address(uint32_t remote_address) { this->remote_address_ = remote_address; }
  void set_storage_namespace(const char *storage_namespace) {
    this->storage_namespace_ = storage_namespace;
  }
  void set_storage_key(const char *storage_key) {
    this->storage_key_ = storage_key;
  }
  void set_repeat(int repeat) { this->repeat_ = repeat; }
  
  void set_frequency(double freq) { this->freq_ = freq; }
  void set_cc1101_module(cc1101::CC1101 *cc1101_module) { this->cc1101_module_ = cc1101_module; }
  void register_program_button(button::Button *button) {
	  button->add_on_press_callback([this] { return this->program(); });
  }
};

} // namespace somfy
} // namespace esphome
