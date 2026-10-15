# ESPHome Enjoy cover remote

This repository contains an ESPHome external component for transmitting Enjoy
blind commands with a remote transmitter such as a CC1101. The `enjoy_motors:`
component handles RF commands and rolling-code storage; ESPHome's standard
`time_based` cover handles position estimation.

## Example

```yaml
external_components:
  - source: github://markstor/esphome-configs
    components: [enjoy_motors]

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

cc1101:
  id: cc1101_id0
  cs_pin: 5
  gdo0_pin:
    number: 16
    allow_other_uses: true
  frequency: 433.92MHz
  modulation_type: ASK/OOK

remote_transmitter:
  id: rf_transmitter
  pin:
    number: 16
    allow_other_uses: true
  carrier_duty_percent: 100%
  non_blocking: false
  on_transmit:
    then:
      - cc1101.begin_tx: cc1101_id0
  on_complete:
    then:
      - cc1101.begin_rx: cc1101_id0

button:
  - platform: template
    id: prog_enjoy_0
    name: "Enjoy Program"

enjoy_motors:
  - id: enjoy_blind_0
    number: 0
    remote_address: !secret enjoy_remote_address
    remote_transmitter: rf_transmitter
    prog_button: prog_enjoy_0

cover:
  - platform: time_based
    id: enjoy_cover_0
    name: "Enjoy Blind"
    device_class: shutter
    open_duration: 25.4s
    close_duration: 24.9s
    open_action:
      - enjoy_motors.open: enjoy_blind_0
    close_action:
      - enjoy_motors.close: enjoy_blind_0
    stop_action:
      - enjoy_motors.stop: enjoy_blind_0
```

Add one `enjoy_motors:` entry per channel. Set `number` from 0 to 3. To send a
command to multiple channels, set `is_multiple: true`. The component does not set the
CC1101 frequency; it uses the frequency configured on the radio.

## Enjoy component options

| Option | Required | Default | Description |
| --- | --- | --- | --- |
| `id` | Yes | | ID used by Enjoy actions. |
| `remote_transmitter` | Yes | | ID of the `remote_transmitter` used to send the waveform. |
| `remote_address` | Yes | | Enjoy remote address, from `0` to `4095`. |
| `number` | Yes | | Blind channel, from `0` to `3`. |
| `is_multiple` | No | `false` | Send to multiple channels instead of only `number`. |
| `storage_namespace` | No | `EnjoyCodes` | ESP32 Preferences namespace for the rolling counter (maximum 15 characters). |
| `storage_key` | No | `mandoEnjoy` | Preferences key for the rolling counter (maximum 15 characters). |
| `repeat` | No | `1` | Repeated frames after the initial frame (from `1` to `16`). |
| `prog_button` | No | | Optional button ID that sends the Enjoy PROG command when pressed. |

The rolling counter is shared by configurations that use the same Preferences
namespace and key. Use the same pair for channels controlled by the same Enjoy
remote address. Existing `cover: - platform: enjoy` configurations used the
old `storage_namespace` value as the Preferences key; when migrating, move that
value to `storage_key` and keep `storage_namespace: "EnjoyCodes"` to preserve
the saved counter.

## Actions

The commands can be called directly from automations or from a
`time_based` cover:

- `enjoy_motors.open`
- `enjoy_motors.close`
- `enjoy_motors.stop`
- `enjoy_motors.program`
- `enjoy_motors.send` for a selected command and optional repeat count

For example, a template button can send PROG without attaching it through
`prog_button`:

```yaml
button:
  - platform: template
    name: "Enjoy Program"
    on_press:
      - enjoy_motors.program: enjoy_blind_0
```

`enjoy_motors.send` accepts `UP`, `DOWN`, `STOP`, or `PROG`; `repeat_count` may be a
literal value or a lambda result from `1` to `16`.

## Hardware

Typical hardware includes an ESP32 and a CC1101 RF transceiver module. Configure
the radio for the frequency and modulation used by your Enjoy remotes.

## Credits

- [Enjoy_Remote_Lib](https://github.com/markstor/Enjoy_Remote_Lib), which
  provides foundational RF decoding and encoding routines.
- [esphome-cc1101](https://github.com/HarmEllis/esphome-cc1101), for CC1101
  integration examples.

## License

This repository is released under the MIT License. See [LICENSE](LICENSE).
