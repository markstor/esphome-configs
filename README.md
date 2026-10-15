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
  id: "cc1101_id0"
  cs_pin: GPIO5
  frequency: 433.92MHz
  modulation_type: ASK/OOK

remote_transmitter:
  id: "rf_transmitter"
  pin: GPIO16
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
    name: "Enjoy Program"
    on_press:
      - enjoy_motors.program:
          id: enjoy_remote
          number: 1

enjoy_motors:
  - id: enjoy_remote
    remote_address: !secret enjoy_remote_address
    remote_transmitter: rf_transmitter
    storage_key: "mandoEnjoy"

cover:
  - platform: time_based
    id: enjoy_cover_0
    name: "Enjoy Blind"
    device_class: shutter
    open_duration: 25.4s
    close_duration: 24.9s
    open_action:
      - enjoy_motors.open:
          id: enjoy_remote
          number: 1
    close_action:
      - enjoy_motors.close:
          id: enjoy_remote
          number: 1
    stop_action:
      - enjoy_motors.stop:
          id: enjoy_remote
          number: 1
```

Add one `enjoy_motors:` entry per remote address, and use its ID in the actions.
Each action selects a blind with `number` from 1 to 4, matching commercial
remote labels. The component converts this to the protocol's zero-based channel
index. Set `is_multiple: true`
on an action to address all channels. The component does not set the CC1101
frequency; it uses the frequency configured on the radio.

## Enjoy component options

| Option | Required | Default | Description |
| --- | --- | --- | --- |
| `id` | Yes | | ID used by Enjoy actions. |
| `remote_transmitter` | Yes | | ID of the `remote_transmitter` used to send the waveform. |
| `remote_address` | Yes | | Enjoy remote address, from `0` to `4095`. |
| `storage_namespace` | No | `EnjoyCodes` | ESP32 Preferences namespace for the rolling counter (maximum 15 characters). |
| `storage_key` | Yes | | Preferences key for this remote's rolling counter (maximum 15 characters). |
| `repeat` | No | `1` | Repeated frames after the initial frame (from `1` to `16`). |

Configure one sender per remote address and give each remote address a unique
`storage_key`. That sender's counter is shared by all actions that select a
blind or use multicast. To preserve a counter from the old Enjoy cover
component, use the old `storage_namespace` value as the new `storage_key` (the
old default was `mandoEnjoy`) and keep `storage_namespace: "EnjoyCodes"`.

## Action options

All actions require the sender `id` and a `number` from `1` to `4`, matching
the channel labels on commercial remotes.
`is_multiple` is optional and defaults to `false`.

Sixteen-channel remotes are not confirmed as supported by this implementation.
The RF frame currently uses the four-channel selection/multicast encoding;
support for other channel layouts needs protocol verification.

## Actions

The commands can be called directly from automations or from a
`time_based` cover:

- `enjoy_motors.open`
- `enjoy_motors.close`
- `enjoy_motors.stop`
- `enjoy_motors.program`
- `enjoy_motors.send` for a selected command and optional repeat count

For example, a template button can send PROG without attaching it through
component configuration:

```yaml
button:
  - platform: template
    name: "Enjoy Program"
    on_press:
      - enjoy_motors.program:
          id: enjoy_remote
          number: 1
```

Set `is_multiple: true` on a command to send it to all channels, for example:

```yaml
- enjoy_motors.close:
    id: enjoy_remote
    number: 1
    is_multiple: true
```

`enjoy_motors.send` accepts `UP`, `DOWN`, `STOP`, or `PROG`; `repeat_count`
may be a literal value or a lambda result from `1` to `16`.

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
