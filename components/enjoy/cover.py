import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import button, cc1101, cover

CODEOWNERS = ["@markstor"]

DEPENDENCIES = ["esp32", "cc1101"]

enjoy_ns = cg.esphome_ns.namespace("enjoy")
EnjoyCover = enjoy_ns.class_("EnjoyCover", cover.Cover, cg.Component)

CONF_ENJOY_REMOTE_ADDRESS = "remote_address"
CONF_CC1101_MODULE = "cc1101_id"
CONF_ENJOY_STORAGE_KEY = "storage_namespace"
CONF_ENJOY_REPEAT = "repeat"
CONF_ENJOY_BLIND_NUMBER = "number"
CONF_ENJOY_MULTICAST = "is_multiple"
CONF_ENJOY_FREQ = "frequency"
CONF_ENJOY_PROG_BUTTON = "program_button_id"

CONFIG_SCHEMA = cover.cover_schema(EnjoyCover).extend(
    {
        cv.Required(CONF_CC1101_MODULE): cv.use_id(cc1101.CC1101),
        cv.Required(CONF_ENJOY_BLIND_NUMBER): cv.int_,
        cv.Required(CONF_ENJOY_REMOTE_ADDRESS): cv.int_,
        cv.Optional(CONF_ENJOY_MULTICAST, default=False): cv.boolean,
        cv.Optional(CONF_ENJOY_STORAGE_KEY, default="mandoEnjoy"): cv.All(
            cv.string, cv.Length(max=15)
        ),
        cv.Optional(CONF_ENJOY_REPEAT, default=1): cv.int_range(min=1, max=16),
        cv.Optional(CONF_ENJOY_FREQ, default=433.92): cv.float_range(min=391, max=464),
        cv.Optional(CONF_ENJOY_PROG_BUTTON): cv.use_id(button.Button),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    cg.add_library("Preferences", None)
    cg.add_library("EnjoyRemoteLib", "2.4.0")
    var = await cover.new_cover(config)
    await cg.register_component(var, config)

    cc1101_module = await cg.get_variable(config[CONF_CC1101_MODULE])
    cg.add(var.set_cc1101_module(cc1101_module))

    cg.add(var.set_blind_number(config[CONF_ENJOY_BLIND_NUMBER]))
    cg.add(var.set_remote_address(config[CONF_ENJOY_REMOTE_ADDRESS]))
    cg.add(var.set_multicast(config[CONF_ENJOY_MULTICAST]))
    cg.add(var.set_storage_key(config[CONF_ENJOY_STORAGE_KEY]))
    cg.add(var.set_repeat(config[CONF_ENJOY_REPEAT]))
    cg.add(var.set_frequency(config[CONF_ENJOY_FREQ]))
    
    if CONF_ENJOY_PROG_BUTTON in config:
        btn = await cg.get_variable(config[CONF_ENJOY_PROG_BUTTON])
        cg.add(var.register_program_button(btn))
