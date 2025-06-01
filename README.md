# ESPHome Configs

This repository contains a collection of [ESPHome](https://esphome.io/) configuration files and custom components for various home automation devices, with a focus on remote control and integration of devices such as blinds, covers, and other RF-based hardware.

## Overview

The configurations and custom components in this repo are designed to simplify the integration of RF-controlled devices with popular home automation systems like Home Assistant using ESPHome. The solutions here are especially useful for scenarios involving Somfy blinds and other similar RF devices.

## Features

- Ready-to-use ESPHome YAML configurations for common RF-controlled devices.
- Custom components for advanced RF communication and cover control.
- Example wiring diagrams and pinouts for supported hardware.
- Modular structure for easy adaptation and extension to new devices.

## Credits & Base Libraries

This repository is based on and incorporates significant work from the following projects:

- **[Enjoy_Remote_Lib](https://github.com/markstor/Enjoy_Remote_Lib) by [markstor](https://github.com/markstor)**  
  Many of the RF decoding and encoding routines originate from this library, which provides foundational support for handling Enjoy remote protocols.

- **[esphome-somfy-cover-remote](https://github.com/HarmEllis/esphome-somfy-cover-remote) by [HarmEllis](https://github.com/HarmEllis)**  
  This project provided inspiration and code for integrating Somfy RTS covers with ESPHome and Home Assistant.

- **[esphome-cc1101](https://github.com/HarmEllis/esphome-cc1101) by [HarmEllis](https://github.com/HarmEllis)**  
  This repository offered valuable resources for using the CC1101 RF transceiver with ESPHome, enabling reliable RF communication in these configs.

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
