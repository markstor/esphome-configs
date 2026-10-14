#pragma once

#include "esphome/components/cover/cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/component.h"
#include <Preferences.h>
#include <array>
#include <vector>


namespace esphome {
namespace somfy {

using namespace esphome::cover;

static const char *const TAG = "somfy.cover";
static constexpr uint16_t SYMBOL_US = 640;

enum class SomfyCommand : uint8_t {
  My = 0x1,
  Up = 0x2,
  Down = 0x4,
  Prog = 0x8,
};

class SomfyCover : public Cover, public Component {
protected:
  double freq_;
  cc1101::CC1101Component *cc1101_module_;
  remote_base::RemoteTransmitterBase *remote_transmitter_;
  const char *storage_namespace_;
  const char *storage_key_;
  uint32_t remote_address_;
  int repeat_;

public:
  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void sendCC1101Command(SomfyCommand command) {
    std::array<uint8_t, 7> frame{};
    const uint16_t rolling_code = this->next_rolling_code_();
    this->build_frame_(frame, command, rolling_code);
    std::vector<int32_t> timings;
    timings.reserve(180 * (this->repeat_ + 1));
    this->append_frame_(timings, frame, 2);
    for (int repeat = 0; repeat < this->repeat_; repeat++) {
      this->append_frame_(timings, frame, 7);
    }

    ESP_LOGI(TAG, "Sending command %01x at %0.2fMHz with rolling code %u", static_cast<uint8_t>(command),
         this->freq_, static_cast<unsigned>(rolling_code));
    this->cc1101_module_->set_frequency(this->freq_ * 1e6f);
    auto call = this->remote_transmitter_->transmit();
    call.get_data()->set_data(timings);
    call.perform();
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI(TAG, "OPEN");
        sendCC1101Command(SomfyCommand::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI(TAG, "CLOSE");
        sendCC1101Command(SomfyCommand::Down);
      } else {
        ESP_LOGI(TAG, "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI(TAG, "STOP");
      sendCC1101Command(SomfyCommand::My);
    }
  }

  void program() {
    ESP_LOGI(TAG, "PROG");
    sendCC1101Command(SomfyCommand::Prog);
  }

  uint16_t next_rolling_code_() {
    ::Preferences preferences;
    if (!preferences.begin(this->storage_namespace_, false)) {
      ESP_LOGE(TAG, "Failed to open rolling-code namespace '%s'", this->storage_namespace_);
      return 1;
    }

    const uint16_t code = preferences.getUShort(this->storage_key_, 1);
    preferences.putUShort(this->storage_key_, code + 1);
    preferences.end();
    return code;
  }

  void build_frame_(std::array<uint8_t, 7> &frame, SomfyCommand command, uint16_t rolling_code) {
    frame[0] = 0xA7;
    frame[1] = static_cast<uint8_t>(static_cast<uint8_t>(command) << 4);
    frame[2] = static_cast<uint8_t>(rolling_code >> 8);
    frame[3] = static_cast<uint8_t>(rolling_code);
    frame[4] = static_cast<uint8_t>(this->remote_address_ >> 16);
    frame[5] = static_cast<uint8_t>(this->remote_address_ >> 8);
    frame[6] = static_cast<uint8_t>(this->remote_address_);

    uint8_t checksum = 0;
    for (uint8_t byte : frame) {
      checksum ^= byte ^ (byte >> 4);
    }
    frame[1] |= checksum & 0x0F;
    for (size_t index = 1; index < frame.size(); index++) {
      frame[index] ^= frame[index - 1];
    }
  }

  static void append_mark_(std::vector<int32_t> &timings, uint32_t duration) {
    timings.push_back(static_cast<int32_t>(duration));
  }

  static void append_space_(std::vector<int32_t> &timings, uint32_t duration) {
    timings.push_back(-static_cast<int32_t>(duration));
  }

  static void append_frame_(std::vector<int32_t> &timings, const std::array<uint8_t, 7> &frame, uint8_t sync_count) {
    if (sync_count == 2) {
      append_mark_(timings, 9415);
      append_space_(timings, 89565);
    }
    for (uint8_t sync = 0; sync < sync_count; sync++) {
      append_mark_(timings, 4 * SYMBOL_US);
      append_space_(timings, 4 * SYMBOL_US);
    }
    append_mark_(timings, 4550);
    append_space_(timings, SYMBOL_US);
    for (uint8_t bit = 0; bit < 56; bit++) {
      const bool value = (frame[bit / 8] >> (7 - (bit % 8))) & 1;
      if (value) {
        append_space_(timings, SYMBOL_US);
        append_mark_(timings, SYMBOL_US);
      } else {
        append_mark_(timings, SYMBOL_US);
        append_space_(timings, SYMBOL_US);
      }
    }
    append_space_(timings, 30415);
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
  void set_cc1101_module(cc1101::CC1101Component *cc1101_module) { this->cc1101_module_ = cc1101_module; }
  void set_remote_transmitter(remote_base::RemoteTransmitterBase *transmitter) { this->remote_transmitter_ = transmitter; }
  void register_program_button(button::Button *button) {
	  button->add_on_press_callback([this] { return this->program(); });
  }
};

} // namespace somfy
} // namespace esphome
