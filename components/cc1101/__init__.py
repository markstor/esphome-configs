from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_CLK_PIN,
    CONF_CS_PIN,
    CONF_ID,
    CONF_MISO_PIN,
    CONF_MOSI_PIN,
    PLATFORM_ESP32,
    PLATFORM_ESP8266,
    PLATFORM_RP2040,
)

CODEOWNERS = ["@markstor"]

cc1101_ns = cg.esphome_ns.namespace("cc1101")
CC1101 = cc1101_ns.class_("CC1101", cg.Component)

CONF_CC1101_FREQUENCY = "frequency"
CONF_EMITTER_PIN = "emitter_pin"

def get_platform_defaults():
    # Define platform-specific default pins
    if CORE.is_esp32:
        return {
            "clk_pin": 18,
            "mosi_pin": 23,
            "miso_pin": 19,
            "cs_pin": 5
        }
    elif CORE.is_esp8266:
        return {
            "clk_pin": 14,
            "mosi_pin": 13,
            "miso_pin": 12,
            "cs_pin": 15
        }
    elif CORE.is_rp2040:
        return {
            "clk_pin": 6,
            "mosi_pin": 7,
            "miso_pin": 4,
            "cs_pin": 5,
        }
    else:
        raise cv.Invalid("Unsupported platform")

defaults = get_platform_defaults()

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CC1101),
            cv.Required(CONF_EMITTER_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_CLK_PIN, default=defaults["clk_pin"]): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_MISO_PIN, default=defaults["miso_pin"]): pins.internal_gpio_input_pin_schema,
            cv.Optional(CONF_MOSI_PIN, default=defaults["mosi_pin"]): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_CS_PIN, default=defaults["cs_pin"]): pins.internal_gpio_output_pin_schema,
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

    clk_pin = await cg.gpio_pin_expression(config[CONF_CLK_PIN])
    cg.add(var.set_clk_pin(clk_pin))

    miso_pin = await cg.gpio_pin_expression(config[CONF_MISO_PIN])
    cg.add(var.set_miso_pin(miso_pin))

    mosi_pin = await cg.gpio_pin_expression(config[CONF_MOSI_PIN])
    cg.add(var.set_mosi_pin(mosi_pin))

    cs_pin = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(cs_pin))

    emitter_pin = await cg.gpio_pin_expression(config[CONF_EMITTER_PIN])
    cg.add(var.set_emitter_pin(emitter_pin))

    cg.add(var.set_cc1101_frequency(config[CONF_CC1101_FREQUENCY]))
