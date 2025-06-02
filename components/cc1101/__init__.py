from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    PLATFORM_ESP32,
    PLATFORM_ESP8266,
    PLATFORM_RP2040,
)

CODEOWNERS = ["@markstor"]

cc1101_ns = cg.esphome_ns.namespace("cc1101")
CC1101 = cc1101_ns.class_("CC1101", cg.Component)

CONF_CC1101_FREQUENCY = "frequency"
CONF_EMITTER_PIN = "emitter_pin"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CC1101),
            cv.Required(CONF_EMITTER_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_CC1101_FREQUENCY, default=433.92): cv.float_range(min=300, max=928),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on([PLATFORM_ESP32, PLATFORM_ESP8266, PLATFORM_RP2040]),
    cv.only_with_arduino,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add_library("SPI", None)
    cg.add_library("EEPROM", None)
    cg.add_library("SmartRC-CC1101-Driver-Lib", None)
    await cg.register_component(var, config)

    emitter_pin = await cg.gpio_pin_expression(config[CONF_EMITTER_PIN])
    cg.add(var.set_emitter_pin(emitter_pin))

    cg.add(var.set_cc1101_frequency(config[CONF_CC1101_FREQUENCY]))
