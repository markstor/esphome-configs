#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/component.h"
#include <Preferences.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace esphome {
namespace enjoy {

using namespace esphome::cover;

static const char *const TAG = "enjoy.cover";
static constexpr uint16_t SYMBOL_US = 418;

enum class EnjoyCommand : uint8_t {
  Up = 0x8,
  Stop = 0x4,
  Down = 0x2,
  Prog = 0x1,
};

class EnjoyCover : public Cover, public Component {
 protected:
  double freq_;
  cc1101::CC1101Component *cc1101_module_;
  remote_base::RemoteTransmitterBase *remote_transmitter_;
  const char *storage_key_;
  uint32_t remote_address_;
  int blind_number_;
  bool multicast_;
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

  void sendCC1101Command(EnjoyCommand command) {
    const uint8_t rolling_code = this->next_rolling_code_();
    std::array<uint8_t, 8> frame{};
    this->build_frame_(frame, command, rolling_code);

    std::vector<int32_t> timings;
    timings.reserve(280 * (this->repeat_ + 1));
    this->append_frame_(timings, frame, true);
    for (int repeat = 0; repeat < this->repeat_; repeat++) {
      this->append_space_(timings, 26800);
      this->append_frame_(timings, frame, false);
    }

    ESP_LOGI(TAG, "Sending command %01x at %0.2fMHz to blind %d. Counter: %u. Multicast: %d. Repeat: %d",
             static_cast<uint8_t>(command), this->freq_, this->blind_number_, rolling_code, this->multicast_, this->repeat_);
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

  uint8_t next_rolling_code_() {
    Preferences preferences;
    preferences.begin("EnjoyCodes", false);
    const uint8_t code = preferences.getUChar(this->storage_key_, 0) + 1;
    preferences.putUChar(this->storage_key_, code);
    preferences.end();
    return code;
  }

  void build_frame_(std::array<uint8_t, 8> &frame, EnjoyCommand command, uint8_t code) {
    const uint8_t button = static_cast<uint8_t>(command);
    frame[1] = this->multicast_ ? 0x0F : static_cast<uint8_t>(1U << this->blind_number_);
    frame[2] = static_cast<uint8_t>((button << 4) | ((this->remote_address_ & 0xF00) >> 8));
    frame[3] = static_cast<uint8_t>(this->remote_address_ & 0xFF);
    frame[5] = static_cast<uint8_t>(this->blind_number_);
    frame[6] = static_cast<uint8_t>(((frame[2] & 0xF0) | (frame[5] ^ 0x01)) ^ code);

    uint8_t remainder = 0;
    for (size_t byte = 0; byte < 7; byte++) {
      remainder ^= frame[byte];
      for (uint8_t bit = 0; bit < 8; bit++) {
        remainder = (remainder & 0x80) ? static_cast<uint8_t>((remainder << 1) ^ 0x07)
                                       : static_cast<uint8_t>(remainder << 1);
      }
    }
    frame[7] = remainder;
  }

  static void append_mark_(std::vector<int32_t> &timings, uint32_t duration) {
    timings.push_back(static_cast<int32_t>(duration));
  }

  static void append_space_(std::vector<int32_t> &timings, uint32_t duration) {
    timings.push_back(-static_cast<int32_t>(duration));
  }

  static void append_frame_(std::vector<int32_t> &timings, const std::array<uint8_t, 8> &frame, bool first) {
    append_mark_(timings, 370);
    append_space_(timings, 2500);
    const uint8_t sync_count = first ? 64 : 24;
    for (uint8_t sync = 0; sync < sync_count; sync++) {
      append_space_(timings, 400);
      append_mark_(timings, 400);
    }
    append_space_(timings, 4000);
    append_mark_(timings, 400);
    append_space_(timings, 400);
    for (uint8_t bit = 0; bit < 64; bit++) {
      const bool value = (frame[bit / 8] >> (7 - (bit % 8))) & 1;
      if (value) {
        append_mark_(timings, SYMBOL_US);
        append_space_(timings, SYMBOL_US);
      } else {
        append_space_(timings, SYMBOL_US);
        append_mark_(timings, SYMBOL_US);
      }
    }
  }

  void set_remote_address(uint32_t remote_address) { this->remote_address_ = remote_address; }
  void set_storage_key(const char *storage_key) { this->storage_key_ = storage_key; }
  void set_blind_number(int blind_number) { this->blind_number_ = blind_number; }
  void set_repeat(int repeat) { this->repeat_ = repeat; }
  void set_multicast(bool multicast) { this->multicast_ = multicast; }
  void set_frequency(double freq) { this->freq_ = freq; }
  void set_cc1101_module(cc1101::CC1101Component *cc1101_module) { this->cc1101_module_ = cc1101_module; }
  void set_remote_transmitter(remote_base::RemoteTransmitterBase *transmitter) { this->remote_transmitter_ = transmitter; }
  void register_program_button(button::Button *button) {
    button->add_on_press_callback([this] { return this->program(); });
  }
};

}  // namespace enjoy
}  // namespace esphome
