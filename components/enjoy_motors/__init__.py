import esphome.codegen as cg
from esphome import automation
from esphome.components import remote_transmitter
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@markstor"]
DEPENDENCIES = ["esp32", "remote_transmitter"]
MULTI_CONF = True

enjoy_motors_ns = cg.esphome_ns.namespace("enjoy_motors")
EnjoyRemote = enjoy_motors_ns.class_("EnjoyRemote", cg.Component)
EnjoyCommand = enjoy_motors_ns.enum("EnjoyCommand", is_class=True)
SendAction = enjoy_motors_ns.class_("SendAction", automation.Action)

COMMAND_MAP = {
    "UP": EnjoyCommand.Up,
    "DOWN": EnjoyCommand.Down,
    "STOP": EnjoyCommand.Stop,
    "PROG": EnjoyCommand.Prog,
}

CONF_REMOTE_TRANSMITTER = "remote_transmitter"
CONF_REMOTE_ADDRESS = "remote_address"
CONF_NUMBER = "number"
CONF_IS_MULTIPLE = "is_multiple"
CONF_STORAGE_NAMESPACE = "storage_namespace"
CONF_STORAGE_KEY = "storage_key"
CONF_REPEAT = "repeat"
CONF_COMMAND = "command"
CONF_REPEAT_COUNT = "repeat_count"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EnjoyRemote),
        cv.Required(CONF_REMOTE_TRANSMITTER): cv.use_id(
            remote_transmitter.RemoteTransmitterComponent
        ),
        cv.Required(CONF_REMOTE_ADDRESS): cv.int_range(min=0, max=0xFFF),
        cv.Required(CONF_STORAGE_KEY): cv.All(cv.string, cv.Length(min=1, max=15)),
        cv.Optional(CONF_STORAGE_NAMESPACE, default="EnjoyCodes"): cv.All(
            cv.string, cv.Length(max=15)
        ),
        cv.Optional(CONF_REPEAT, default=1): cv.int_range(min=1, max=16),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    transmitter = await cg.get_variable(config[CONF_REMOTE_TRANSMITTER])
    cg.add(var.set_remote_transmitter(transmitter))
    cg.add(var.set_remote_address(config[CONF_REMOTE_ADDRESS]))
    cg.add(var.set_storage_namespace(config[CONF_STORAGE_NAMESPACE]))
    cg.add(var.set_storage_key(config[CONF_STORAGE_KEY]))
    cg.add(var.set_repeat(config[CONF_REPEAT]))


ENJOY_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(EnjoyRemote),
        cv.Required(CONF_NUMBER): cv.int_range(min=1, max=4),
        cv.Optional(CONF_IS_MULTIPLE, default=False): cv.boolean,
    }
)


async def _enjoy_action_to_code(config, action_id, template_arg, command):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    cg.add(var.set_command(command))
    cg.add(var.set_blind_number(config[CONF_NUMBER] - 1))
    cg.add(var.set_multicast(config[CONF_IS_MULTIPLE]))
    return var


@automation.register_action(
    "enjoy_motors.open", SendAction, ENJOY_ACTION_SCHEMA, synchronous=True
)
async def enjoy_open_action_to_code(config, action_id, template_arg, args):
    return await _enjoy_action_to_code(config, action_id, template_arg, EnjoyCommand.Up)


@automation.register_action(
    "enjoy_motors.close", SendAction, ENJOY_ACTION_SCHEMA, synchronous=True
)
async def enjoy_close_action_to_code(config, action_id, template_arg, args):
    return await _enjoy_action_to_code(config, action_id, template_arg, EnjoyCommand.Down)


@automation.register_action(
    "enjoy_motors.stop", SendAction, ENJOY_ACTION_SCHEMA, synchronous=True
)
async def enjoy_stop_action_to_code(config, action_id, template_arg, args):
    return await _enjoy_action_to_code(config, action_id, template_arg, EnjoyCommand.Stop)


@automation.register_action(
    "enjoy_motors.program", SendAction, ENJOY_ACTION_SCHEMA, synchronous=True
)
async def enjoy_program_action_to_code(config, action_id, template_arg, args):
    return await _enjoy_action_to_code(config, action_id, template_arg, EnjoyCommand.Prog)


ENJOY_SEND_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(EnjoyRemote),
        cv.Required(CONF_NUMBER): cv.int_range(min=1, max=4),
        cv.Optional(CONF_IS_MULTIPLE, default=False): cv.boolean,
        cv.Required(CONF_COMMAND): cv.enum(COMMAND_MAP, upper=True),
        cv.Optional(CONF_REPEAT_COUNT): cv.templatable(
            cv.int_range(min=1, max=16)
        ),
    }
)


@automation.register_action(
    "enjoy_motors.send", SendAction, ENJOY_SEND_ACTION_SCHEMA, synchronous=True
)
async def enjoy_send_action_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    cg.add(var.set_command(config[CONF_COMMAND]))
    cg.add(var.set_blind_number(config[CONF_NUMBER] - 1))
    cg.add(var.set_multicast(config[CONF_IS_MULTIPLE]))
    if CONF_REPEAT_COUNT in config:
        repeat_count = await cg.templatable(config[CONF_REPEAT_COUNT], args, cg.int_)
        cg.add(var.set_repeat_count(repeat_count))
    return var
