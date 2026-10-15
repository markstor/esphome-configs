#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <Preferences.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace esphome {
namespace enjoy_motors {

static const char *const TAG = "enjoy";
static constexpr uint16_t SYMBOL_US = 418;

enum class EnjoyCommand : uint8_t {
  Up = 0x8,
  Stop = 0x4,
  Down = 0x2,
  Prog = 0x1,
};

class EnjoyRemote : public Component {
 public:
  void setup() override {
    if (this->prog_button_ != nullptr) {
      this->prog_button_->add_on_press_callback([this] { this->program(); });
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "Enjoy command component:");
    ESP_LOGCONFIG(TAG, "  Remote address: 0x%03X", this->remote_address_);
    ESP_LOGCONFIG(TAG, "  Blind number: %d", this->blind_number_);
    ESP_LOGCONFIG(TAG, "  Multicast: %s", YESNO(this->multicast_));
    ESP_LOGCONFIG(TAG, "  Repeat count: %d", this->repeat_);
    ESP_LOGCONFIG(TAG, "  Storage namespace: %s", this->storage_namespace_);
    ESP_LOGCONFIG(TAG, "  Storage key: %s", this->storage_key_);
    ESP_LOGCONFIG(TAG, "  Program button attached: %s", YESNO(this->prog_button_ != nullptr));
  }

  void set_remote_transmitter(remote_base::RemoteTransmitterBase *transmitter) {
    this->remote_transmitter_ = transmitter;
  }
  void set_prog_button(button::Button *button) { this->prog_button_ = button; }
  void set_remote_address(uint32_t remote_address) { this->remote_address_ = remote_address; }
  void set_blind_number(int blind_number) { this->blind_number_ = blind_number; }
  void set_multicast(bool multicast) { this->multicast_ = multicast; }
  void set_storage_namespace(const char *storage_namespace) { this->storage_namespace_ = storage_namespace; }
  void set_storage_key(const char *storage_key) { this->storage_key_ = storage_key; }
  void set_repeat(int repeat) { this->repeat_ = repeat; }

  void open() { this->send_command(EnjoyCommand::Up); }
  void close() { this->send_command(EnjoyCommand::Down); }
  void stop() { this->send_command(EnjoyCommand::Stop); }
  void program() { this->send_command(EnjoyCommand::Prog); }

  void send_command(EnjoyCommand command) { this->send_command(command, this->repeat_); }

  void send_command(EnjoyCommand command, int repeat) {
    if (this->remote_transmitter_ == nullptr) {
      ESP_LOGE(TAG, "No remote_transmitter configured");
      return;
    }
    if (repeat < 1 || repeat > 16) {
      ESP_LOGE(TAG, "Repeat count %d is outside the supported range 1-16", repeat);
      return;
    }

    uint8_t rolling_code;
    if (!this->next_rolling_code_(&rolling_code)) {
      return;
    }

    std::array<uint8_t, 8> frame{};
    this->build_frame_(frame, command, rolling_code);

    std::vector<int32_t> timings;
    timings.reserve(280 * (repeat + 1));
    this->append_frame_(timings, frame, true);
    for (int index = 0; index < repeat; index++) {
      this->append_space_(timings, 26800);
      this->append_frame_(timings, frame, false);
    }

    ESP_LOGI(TAG,
             "Sending command %01x to blind %d. Counter: %u. Multicast: %d. Repeat: %d",
             static_cast<uint8_t>(command), this->blind_number_, rolling_code, this->multicast_, repeat);
    auto call = this->remote_transmitter_->transmit();
    call.get_data()->set_data(timings);
    call.perform();
  }

 protected:
  remote_base::RemoteTransmitterBase *remote_transmitter_{nullptr};
  button::Button *prog_button_{nullptr};
  const char *storage_namespace_{"EnjoyCodes"};
  const char *storage_key_{"mandoEnjoy"};
  uint32_t remote_address_{0};
  int blind_number_{0};
  bool multicast_{false};
  int repeat_{1};

  bool next_rolling_code_(uint8_t *code) {
    Preferences preferences;
    if (!preferences.begin(this->storage_namespace_, false)) {
      ESP_LOGE(TAG, "Failed to open preferences namespace '%s'", this->storage_namespace_);
      return false;
    }

    *code = static_cast<uint8_t>(preferences.getUChar(this->storage_key_, 0) + 1);
    const size_t bytes_written = preferences.putUChar(this->storage_key_, *code);
    preferences.end();
    if (bytes_written != sizeof(*code)) {
      ESP_LOGE(TAG, "Failed to persist rolling code using key '%s'", this->storage_key_);
      return false;
    }
    return true;
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
};

template<typename... Ts> class SendAction : public Action<Ts...> {
 public:
  explicit SendAction(EnjoyRemote *parent) : parent_(parent) {}

  void set_command(EnjoyCommand command) { this->command_ = command; }
  TEMPLATABLE_VALUE(int, repeat_count)

  void play(const Ts &...x) override {
    if (this->repeat_count_.has_value()) {
      this->parent_->send_command(this->command_, this->repeat_count_.value(x...));
    } else {
      this->parent_->send_command(this->command_);
    }
  }

 protected:
  EnjoyRemote *parent_;
  EnjoyCommand command_{EnjoyCommand::Stop};
};

}  // namespace enjoy_motors
}  // namespace esphome
