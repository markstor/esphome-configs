import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import button, cc1101, cover, remote_transmitter

CODEOWNERS = ["@markstor"]

DEPENDENCIES = ["esp32", "cc1101", "remote_transmitter"]

somfy_ns = cg.esphome_ns.namespace("somfy")
SomfyCover = somfy_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONF_SOMFY_REMOTE_ADDRESS = "remote_address"
CONF_CC1101_MODULE = "cc1101_id"
CONF_REMOTE_TRANSMITTER = "remote_transmitter_id"
CONF_SOMFY_PIN = "pin"
CONF_SOMFY_STORAGE_KEY = "storage_key"
CONF_SOMFY_STORAGE_NAMESPACE = "storage_namespace"
CONF_SOMFY_REPEAT = "repeat"
CONF_SOMFY_FREQ = "frequency"
CONF_SOMFY_RESTORE_FREQ = "restore_frequency"
CONF_SOMFY_PROG_BUTTON = "program_button_id"

CONFIG_SCHEMA = cover.cover_schema(SomfyCover).extend(
    {
        cv.Required(CONF_CC1101_MODULE): cv.use_id(cc1101.CC1101Component),
        cv.Required(CONF_REMOTE_TRANSMITTER): cv.use_id(
            remote_transmitter.RemoteTransmitterComponent
        ),
        cv.Required(CONF_SOMFY_REMOTE_ADDRESS): cv.int_,
        cv.Required(CONF_SOMFY_STORAGE_KEY): cv.All(cv.string, cv.Length(max=15)),
        cv.Optional(CONF_SOMFY_STORAGE_NAMESPACE, default="somfy"): cv.All(
            cv.string, cv.Length(max=15)
        ),
        cv.Optional(CONF_SOMFY_REPEAT, default=4): cv.int_range(min=1, max=16),
        cv.Optional(CONF_SOMFY_FREQ, default=433.42): cv.float_range(min=391, max=464),
        cv.Optional(CONF_SOMFY_RESTORE_FREQ, default=433.92): cv.float_range(min=391, max=464),
        cv.Optional(CONF_SOMFY_PROG_BUTTON): cv.use_id(button.Button),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    cg.add_library("Preferences", None)
    
    var = await cover.new_cover(config)
    await cg.register_component(var, config)
    
    cc1101_module = await cg.get_variable(config[CONF_CC1101_MODULE])
    cg.add(var.set_cc1101_module(cc1101_module))
    transmitter = await cg.get_variable(config[CONF_REMOTE_TRANSMITTER])
    cg.add(var.set_remote_transmitter(transmitter))

    cg.add(var.set_remote_address(config[CONF_SOMFY_REMOTE_ADDRESS]))
    cg.add(var.set_storage_key(config[CONF_SOMFY_STORAGE_KEY]))
    cg.add(var.set_storage_namespace(config[CONF_SOMFY_STORAGE_NAMESPACE]))
    cg.add(var.set_repeat(config[CONF_SOMFY_REPEAT]))
    cg.add(var.set_frequency(config[CONF_SOMFY_FREQ]))
    cg.add(var.set_restore_frequency(config[CONF_SOMFY_RESTORE_FREQ]))
    if CONF_SOMFY_PROG_BUTTON in config:
        btn = await cg.get_variable(config[CONF_SOMFY_PROG_BUTTON])
        cg.add(var.register_program_button(btn))
