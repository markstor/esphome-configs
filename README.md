# ESPHome Configs

This repository contains a collection of [ESPHome](https://esphome.io/) configuration files and custom components for various home automation devices, with a focus on remote control and integration of devices such as blinds, covers, and other RF-based hardware.

## Overview

The configurations and custom components in this repo are designed to simplify the integration of RF-controlled devices with popular home automation systems like Home Assistant using ESPHome. The solutions here are especially useful for scenarios involving Somfy blinds and other similar RF devices.

## Features

- Ready-to-use ESPHome YAML configurations for common RF-controlled devices.
- Custom components for advanced RF communication and cover control.
- Example wiring diagrams and pinouts for supported hardware.
- Modular structure for easy adaptation and extension to new devices.

## Example configuration

```yaml
cc1101:
  id: "cc1101_id0"
  emitter_pin: 16

button:
  - platform: template
    id: "prog_somfy_1"
    name: "PROG button 1"
  # This button without name won't be exposed in HA
  - platform: template
    id: "prog_enjoy_0"

cover:
  - platform: somfy
    id: "somfy1"
    remote_address: !secret somfy_remote_address_1
    storage_key: "somfy1"
    cc1101_id: "cc1101_id0"
    program_button_id: "prog_somfy_1"
  
  - platform: somfy
    id: "somfy2"
    remote_address: !secret somfy_remote_address_2
    storage_key: "somfy2"
    cc1101_id: "cc1101_id0"
  
  - platform: enjoy
    id: "enjoy1"
    number: 0
    remote_address: !secret enjoy_remote_address
    cc1101_id: "cc1101_id0"
    program_button_id: "prog_enjoy_0"
  
  - platform: enjoy
    id: "enjoy2"
    number: 1
    remote_address: !secret enjoy_remote_address
    cc1101_id: "cc1101_id0"
  
  - platform: enjoy
    id: "enjoy3"
    number: 2
    remote_address: !secret enjoy_remote_address
    cc1101_id: "cc1101_id0"
  
  - platform: enjoy
    id: "enjoy4"
    number: 3
    remote_address: !secret enjoy_remote_address
    cc1101_id: "cc1101_id0"
  
  - platform: enjoy
    id: "multi0"
    name: "Multi Enjoy Blind"
    number: 0
    is_multiple: true
    remote_address: !secret enjoy_remote_address
    cc1101_id: "cc1101_id0"

# we can wrap them as time based covers if desired.
# Remember that only the components with names will be on home assistant
  - platform: time_based
    name: "Enjoy Blind"
    open_action: 
      - cover.open: enjoy1
    close_action:
      - cover.close: enjoy1
    stop_action: 
      - cover.stop: enjoy1
    open_duration: 25.4s
    close_duration: 24.9s
    manual_control: True
      
  - platform: time_based
    name: "Enjoy Blind2"
    open_action: 
      - cover.open: enjoy2
    close_action:
      - cover.close: enjoy2
    stop_action: 
      - cover.stop: enjoy2
    open_duration: 24.4s
    close_duration: 23.7s
    manual_control: True
    
  - platform: time_based
    name: "Enjoy Blind3"
    open_action: 
      - cover.open: enjoy3
    close_action:
      - cover.close: enjoy3
    stop_action: 
      - cover.stop: enjoy3
    open_duration: 16.7s
    close_duration: 16.5s
    manual_control: True
    
  - platform: time_based
    name: "Enjoy Blind4"
    open_action: 
      - cover.open: enjoy4
    close_action:
      - cover.close: enjoy4
    stop_action: 
      - cover.stop: enjoy4
    open_duration: 15.8s
    close_duration: 15.5s
    manual_control: True
```

## Credits & Base Libraries

This repository is based on and incorporates significant work from the following projects:

- **[Enjoy_Remote_Lib](https://github.com/markstor/Enjoy_Remote_Lib) by [me](https://github.com/markstor)**  
  Many of the RF decoding and encoding routines originate from this library, which provides foundational support for handling Enjoy remote protocols.

- **[esphome-somfy-cover-remote](https://github.com/HarmEllis/esphome-somfy-cover-remote) by [HarmEllis](https://github.com/HarmEllis)**  
  This project provided inspiration and code for integrating Somfy RTS covers with ESPHome and Home Assistant.

- **[esphome-cc1101](https://github.com/HarmEllis/esphome-cc1101) by [HarmEllis](https://github.com/HarmEllis)**  
  This repository offered valuable resources for using the CC1101 RF transceiver with ESPHome, enabling reliable RF communication in these configs.

I had to redo some bits to be able to shift frequency bands, as my setup uses the CC1101 module for different things.

## Hardware

Typical hardware supported includes:

- ESP8266/ESP32 boards
- CC1101 RF transceiver modules
- Relays and input buttons for local control

Refer to specific configuration files for wiring diagrams and hardware requirements.

## Contributing

Contributions are welcome! Feel free to submit issues, pull requests, or suggestions for additional device support.

## License

This repository is released under the MIT License. Please see [LICENSE](LICENSE) for details.  
Credit is given to all upstream projects and contributors.
