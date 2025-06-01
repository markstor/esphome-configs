import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import cover
from esphome.const import CONF_ID
from esphome import pins

DEPENDENCIES = ["esp32"]

somfy_ns = cg.esphome_ns.namespace("somfy")
SomfyCover = somfy_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONF_SOMFY_REMOTE_ADDRESS = "remote_address"
CONF_SOMFY_PIN = "pin"
CONF_SOMFY_STORAGE_KEY = "storage_key"
CONF_SOMFY_STORAGE_NAMESPACE = "storage_namespace"
CONF_SOMFY_REPEAT = "repeat"
CONF_SOMFY_FREQ = "frequency"
CONF_SOMFY_PROG_BUTTON = "program_button"


CONFIG_SCHEMA = cover.COVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SomfyCover),
        cv.Required(CONF_CC1101_MODULE): cv.use_id(cc1101.CC1101),
        cv.Required(CONF_SOMFY_REMOTE_ADDRESS): cv.int_,
        cv.Required(CONF_SOMFY_STORAGE_KEY): cv.All(cv.string, cv.Length(max=15)),
        cv.Optional(CONF_SOMFY_STORAGE_NAMESPACE, default="somfy"): cv.All(
            cv.string, cv.Length(max=15)
        ),
        cv.Optional(CONF_SOMFY_REPEAT, default=4): cv.int_range(min=1, max=16),
        cv.Optional(CONF_SOMFY_FREQ, default=433.42): cv.float_range(min=391, max=464),
        cv.Optional(CONF_SOMFY_PROG_BUTTON): cv.All(
            button.BUTTON_SCHEMA.extend({}),
            cv.has_at_most_one_key(CONF_NAME),  # user can override if they want
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    
    cc1101_module = await cg.get_variable(config[CONF_CC1101_MODULE])
    cg.add(var.set_cc1101_module(cc1101_module))

    cg.add(var.set_remote_address(config[CONF_SOMFY_REMOTE_ADDRESS]))
    cg.add(var.set_storage_key(config[CONF_SOMFY_STORAGE_KEY]))
    cg.add(var.set_storage_namespace(config[CONF_SOMFY_STORAGE_NAMESPACE]))
    cg.add(var.set_repeat(config[CONF_SOMFY_REPEAT]))
    cg.add(var.set_frequency(config[CONF_SOMFY_FREQ]))
    if CONF_SOMFY_PROG_BUTTON in config:
        btn_config = config[CONF_SOMFY_PROG_BUTTON]
        if CONF_NAME not in btn_config:
            btn_config[CONF_NAME] = f"Program {config[CONF_NAME]}"
        
        btn = cg.new_Pvariable(var_id + "_prog_btn", var)
        await button.register_button(btn, btn_config)
        cg.add(var.register_program_button(btn))
