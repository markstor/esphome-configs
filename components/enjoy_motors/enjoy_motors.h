#pragma once

#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include <cstdint>
#include <vector>

namespace esphome {
namespace enjoy_motors {

enum class EnjoyCommand : uint8_t {
  Up = 0x8,
  Stop = 0x4,
  Down = 0x2,
  Prog = 0x1,
};

class EnjoyRemote : public Component {
 public:
  void dump_config() override;

  void set_remote_transmitter(remote_base::RemoteTransmitterBase *transmitter);
  void set_remote_address(uint32_t remote_address);
  void set_storage_namespace(const char *storage_namespace);
  void set_storage_key(const char *storage_key);
  void set_repeat(int repeat);

  void send_command(EnjoyCommand command, int blind_number, bool multicast);
  void send_command(EnjoyCommand command, int blind_number, bool multicast, int repeat);

 protected:
  remote_base::RemoteTransmitterBase *remote_transmitter_{nullptr};
  const char *storage_namespace_{"EnjoyCodes"};
  const char *storage_key_{nullptr};
  uint32_t remote_address_{0};
  int repeat_{1};

  bool next_rolling_code_(uint8_t *code);
  void build_frame_(uint8_t *frame, EnjoyCommand command, uint8_t code, int blind_number, bool multicast);
  static void append_mark_(std::vector<int32_t> &timings, uint32_t duration);
  static void append_space_(std::vector<int32_t> &timings, uint32_t duration);
  void append_frame_(std::vector<int32_t> &timings, const uint8_t *frame, bool first);
};

template<typename... Ts> class SendAction : public Action<Ts...> {
 public:
  explicit SendAction(EnjoyRemote *parent) : parent_(parent) {}

  void set_command(EnjoyCommand command) { this->command_ = command; }
  void set_blind_number(int blind_number) { this->blind_number_ = blind_number; }
  void set_multicast(bool multicast) { this->multicast_ = multicast; }
  TEMPLATABLE_VALUE(int, repeat_count)

  void play(const Ts &...x) override {
    if (this->repeat_count_.has_value()) {
      this->parent_->send_command(this->command_, this->blind_number_, this->multicast_,
                                  this->repeat_count_.value(x...));
    } else {
      this->parent_->send_command(this->command_, this->blind_number_, this->multicast_);
    }
  }

 protected:
  EnjoyRemote *parent_;
  int blind_number_{0};
  bool multicast_{false};
  EnjoyCommand command_{EnjoyCommand::Stop};
};

}  // namespace enjoy_motors
}  // namespace esphome
