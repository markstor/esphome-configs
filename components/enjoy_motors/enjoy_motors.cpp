#include "enjoy_motors.h"
#include <esp_err.h>
#include "esphome/core/log.h"
#include <nvs.h>
#include <array>
#include <cstddef>
#include <vector>

namespace esphome {
namespace enjoy_motors {

static const char *const TAG = "enjoy_motors";
static constexpr uint16_t SYMBOL_US = 418;

void EnjoyRemote::dump_config() {
  ESP_LOGCONFIG(TAG, "Enjoy command component:");
  ESP_LOGCONFIG(TAG, "  Remote address: 0x%03X", static_cast<unsigned int>(this->remote_address_));
  ESP_LOGCONFIG(TAG, "  Repeat count: %d", this->repeat_);
  ESP_LOGCONFIG(TAG, "  Storage namespace: %s", this->storage_namespace_);
  ESP_LOGCONFIG(TAG, "  Storage key: %s", this->storage_key_);
}

void EnjoyRemote::set_remote_transmitter(remote_base::RemoteTransmitterBase *transmitter) {
  this->remote_transmitter_ = transmitter;
}

void EnjoyRemote::set_remote_address(uint32_t remote_address) { this->remote_address_ = remote_address; }

void EnjoyRemote::set_storage_namespace(const char *storage_namespace) {
  this->storage_namespace_ = storage_namespace;
}

void EnjoyRemote::set_storage_key(const char *storage_key) { this->storage_key_ = storage_key; }

void EnjoyRemote::set_repeat(int repeat) { this->repeat_ = repeat; }

void EnjoyRemote::send_command(EnjoyCommand command, int blind_number, bool multicast) {
  this->send_command(command, blind_number, multicast, this->repeat_);
}

void EnjoyRemote::send_command(EnjoyCommand command, int blind_number, bool multicast, int repeat) {
  if (this->remote_transmitter_ == nullptr) {
    ESP_LOGE(TAG, "No remote_transmitter configured");
    return;
  }
  if (blind_number < 0 || blind_number > 3) {
    ESP_LOGE(TAG, "Blind number %d is outside the supported range 0-3", blind_number);
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
  this->build_frame_(frame.data(), command, rolling_code, blind_number, multicast);

  std::vector<int32_t> timings;
  timings.reserve(280 * (repeat + 1));
  this->append_frame_(timings, frame.data(), true);
  for (int index = 0; index < repeat; index++) {
    this->append_space_(timings, 26800);
    this->append_frame_(timings, frame.data(), false);
  }

  ESP_LOGI(TAG,
           "Sending command %01x to blind %d. Counter: %u. Multicast: %d. Repeat: %d",
           static_cast<uint8_t>(command), blind_number, rolling_code, multicast, repeat);
  auto call = this->remote_transmitter_->transmit();
  call.get_data()->set_data(timings);
  call.perform();
}

bool EnjoyRemote::next_rolling_code_(uint8_t *code) {
  if (this->storage_key_ == nullptr || this->storage_key_[0] == '\0') {
    ESP_LOGE(TAG, "No storage_key configured");
    return false;
  }

  nvs_handle_t handle;
  esp_err_t err = nvs_open(this->storage_namespace_, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s", this->storage_namespace_, esp_err_to_name(err));
    return false;
  }

  uint8_t stored_code = 0;
  err = nvs_get_u8(handle, this->storage_key_, &stored_code);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGE(TAG, "Failed to read rolling code using key '%s': %s", this->storage_key_, esp_err_to_name(err));
    nvs_close(handle);
    return false;
  }

  const uint8_t next_code = static_cast<uint8_t>(stored_code + 1);
  err = nvs_set_u8(handle, this->storage_key_, next_code);
  if (err == ESP_OK) {
    err = nvs_commit(handle);
  }
  nvs_close(handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to persist rolling code using key '%s': %s", this->storage_key_, esp_err_to_name(err));
    return false;
  }

  *code = next_code;
  return true;
}

void EnjoyRemote::build_frame_(uint8_t *frame, EnjoyCommand command, uint8_t code, int blind_number, bool multicast) {
  const uint8_t button = static_cast<uint8_t>(command);
  frame[1] = multicast ? 0x0F : static_cast<uint8_t>(1U << blind_number);
  frame[2] = static_cast<uint8_t>((button << 4) | ((this->remote_address_ & 0xF00) >> 8));
  frame[3] = static_cast<uint8_t>(this->remote_address_ & 0xFF);
  frame[5] = static_cast<uint8_t>(blind_number);
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

void EnjoyRemote::append_mark_(std::vector<int32_t> &timings, uint32_t duration) {
  timings.push_back(static_cast<int32_t>(duration));
}

void EnjoyRemote::append_space_(std::vector<int32_t> &timings, uint32_t duration) {
  timings.push_back(-static_cast<int32_t>(duration));
}

void EnjoyRemote::append_frame_(std::vector<int32_t> &timings, const uint8_t *frame, bool first) {
  this->append_mark_(timings, 370);
  this->append_space_(timings, 2500);
  const uint8_t sync_count = first ? 64 : 24;
  for (uint8_t sync = 0; sync < sync_count; sync++) {
    this->append_space_(timings, 400);
    this->append_mark_(timings, 400);
  }
  this->append_space_(timings, 4000);
  this->append_mark_(timings, 400);
  this->append_space_(timings, 400);
  for (uint8_t bit = 0; bit < 64; bit++) {
    const bool value = (frame[bit / 8] >> (7 - (bit % 8))) & 1;
    if (value) {
      this->append_mark_(timings, SYMBOL_US);
      this->append_space_(timings, SYMBOL_US);
    } else {
      this->append_space_(timings, SYMBOL_US);
      this->append_mark_(timings, SYMBOL_US);
    }
  }
}

}  // namespace enjoy_motors
}  // namespace esphome
