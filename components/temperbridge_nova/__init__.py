from esphome import automation
import esphome.codegen as cg
from esphome.components import button, cover, sensor, switch, text_sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_AUTOMATION_ID,
    CONF_KEY,
    CONF_ID,
    CONF_NAME,
    CONF_ON_PRESS,
    CONF_THEN,
    CONF_TYPE_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_PULSE,
    STATE_CLASS_MEASUREMENT,
)
from esphome import pins

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["button", "cover", "sensor", "switch", "text_sensor"]

CONF_ANTI_SNORE = "anti_snore"
CONF_BOARD_ID = "board_id"
CONF_BOARD_ID_PINS = "board_id_pins"
CONF_CAPTURE_STATUS_PACKETS = "capture_status_packets"
CONF_CONTROL_BOX_MODEL = "control_box_model"
CONF_FAVORITE_1 = "favorite_1"
CONF_FAVORITE_2 = "favorite_2"
CONF_FLAT = "flat"
CONF_FOOT_ZONE_MASSAGE = "foot_zone_massage"
CONF_KEY_CODE = "key_code"
CONF_HEAD = "head"
CONF_HEAD_LOWER = "head_lower"
CONF_HEAD_PULSE = "head_pulse"
CONF_HEAD_RAISE = "head_raise"
CONF_HEAD_ZONE_MASSAGE = "head_zone_massage"
CONF_LEGS = "legs"
CONF_LEGS_LOWER = "legs_lower"
CONF_LEGS_PULSE = "legs_pulse"
CONF_LEGS_RAISE = "legs_raise"
CONF_LINK_STATE = "link_state"
CONF_LOG_STATUS_PACKETS = "log_status_packets"
CONF_LUMBAR = "lumbar"
CONF_LUMBAR_LOWER = "lumbar_lower"
CONF_LUMBAR_PULSE = "lumbar_pulse"
CONF_LUMBAR_RAISE = "lumbar_raise"
CONF_MASSAGE_WAVE_MODE = "massage_wave_mode"
CONF_MOVEMENT_STATE = "movement_state"
CONF_RX_ENABLE_PIN = "rx_enable_pin"
CONF_STOP = "stop"
CONF_STATUS_0 = "status_0"
CONF_STATUS_7 = "status_7"
CONF_STATUS_PACKET_CAPTURE = "status_packet_capture"
CONF_TOGGLE_LIGHTS = "toggle_lights"
CONF_TV = "tv"
CONF_ZERO_G = "zero_g"

DEFAULT_BOARD_ID_PINS = [4, 5, 6, 7]
DEFAULT_RX_ENABLE_PIN = 10

temperbridge_nova_ns = cg.esphome_ns.namespace("temperbridge_nova")
TemperBridgeNovaComponent = temperbridge_nova_ns.class_(
    "TemperBridgeNovaComponent", cg.Component, uart.UARTDevice
)
TemperBridgeNovaCover = temperbridge_nova_ns.class_("TemperBridgeNovaCover", cover.Cover)
MfpActuator = temperbridge_nova_ns.enum("MfpActuator", is_class=True)
MfpActuatorDirection = temperbridge_nova_ns.enum(
    "MfpActuatorDirection", is_class=True
)
TemperBridgeNovaButton = temperbridge_nova_ns.class_("TemperBridgeNovaButton", button.Button)
TemperBridgeNovaStopButton = temperbridge_nova_ns.class_(
    "TemperBridgeNovaStopButton", button.Button
)
TemperBridgeNovaActuatorButton = temperbridge_nova_ns.class_(
    "TemperBridgeNovaActuatorButton", button.Button
)
TemperBridgeNovaStatusPacketCaptureButton = temperbridge_nova_ns.class_(
    "TemperBridgeNovaStatusPacketCaptureButton", button.Button
)
TemperBridgeNovaStatusPacketLogSwitch = temperbridge_nova_ns.class_(
    "TemperBridgeNovaStatusPacketLogSwitch", switch.Switch
)
TemperBridgeNovaSendKeyAction = temperbridge_nova_ns.class_(
    "TemperBridgeNovaSendKeyAction", automation.Action
)


def _with_default_name(schema, name):
    def set_default_name(value):
        value.setdefault(CONF_NAME, name)
        return value

    return cv.All(set_default_name, schema)


BUTTON_KEY_CODES = {
    CONF_FLAT: "08000000",
    CONF_ZERO_G: "00100000",
    CONF_TV: "00400000",
    CONF_FAVORITE_1: "00000100",
    CONF_FAVORITE_2: "00000080",
    CONF_ANTI_SNORE: "00800000",
    CONF_TOGGLE_LIGHTS: "00000200",
    CONF_MASSAGE_WAVE_MODE: "00000004",
    CONF_HEAD_ZONE_MASSAGE: "00080000",
    CONF_FOOT_ZONE_MASSAGE: "00040000",
}


def _with_send_key_action(config, button_key):
    button_config = dict(config[button_key])
    send_key_automation = automation.validate_automation({})(
        {
            CONF_AUTOMATION_ID: f"temperbridge_nova_{button_key}_send_key_automation",
            CONF_THEN: [
                {
                    CONF_TYPE_ID: f"temperbridge_nova_{button_key}_send_key_action",
                    "temperbridge_nova.send_key": {
                        CONF_ID: config[CONF_ID],
                        CONF_KEY_CODE: BUTTON_KEY_CODES[button_key],
                    }
                }
            ],
        }
    )
    button_config[CONF_ON_PRESS] = [
        *send_key_automation,
        *button_config.get(CONF_ON_PRESS, []),
    ]
    return button_config


async def _new_actuator_button(config, parent, button_key, actuator, direction):
    actuator_button = await button.new_button(config[button_key])
    cg.add(actuator_button.set_parent(parent))
    cg.add(actuator_button.set_actuator(actuator))
    cg.add(actuator_button.set_direction(direction))


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TemperBridgeNovaComponent),
            cv.Optional(CONF_RX_ENABLE_PIN, default=DEFAULT_RX_ENABLE_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_BOARD_ID_PINS, default=DEFAULT_BOARD_ID_PINS): cv.All(
                cv.ensure_list(pins.internal_gpio_input_pullup_pin_schema),
                cv.Length(min=4, max=4),
            ),
            cv.Optional(CONF_BOARD_ID, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon="mdi:identifier",
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Board ID",
            ),
            cv.Optional(CONF_LINK_STATE, default={}): _with_default_name(
                text_sensor.text_sensor_schema(icon="mdi:link-variant"),
                "MFP Link State",
            ),
            cv.Optional(CONF_KEY, default={}): _with_default_name(
                text_sensor.text_sensor_schema(
                    icon="mdi:key",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                ),
                "MFP Key",
            ),
            cv.Optional(CONF_CONTROL_BOX_MODEL, default={}): _with_default_name(
                text_sensor.text_sensor_schema(
                    icon="mdi:chip",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                ),
                "Control Box Model",
            ),
            cv.Optional(CONF_MOVEMENT_STATE, default={}): _with_default_name(
                text_sensor.text_sensor_schema(
                    icon="mdi:motion",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                ),
                "Movement State",
            ),
            cv.Optional(CONF_HEAD_PULSE, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon=ICON_PULSE,
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Head Pulse",
            ),
            cv.Optional(CONF_LEGS_PULSE, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon=ICON_PULSE,
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Legs Pulse",
            ),
            cv.Optional(CONF_LUMBAR_PULSE, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon=ICON_PULSE,
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Lumbar Pulse",
            ),
            cv.Optional(CONF_STATUS_0, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon="mdi:code-brackets",
                    accuracy_decimals=0,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Status[0]",
            ),
            cv.Optional(CONF_STATUS_7, default={}): _with_default_name(
                sensor.sensor_schema(
                    icon="mdi:code-brackets",
                    accuracy_decimals=0,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                "Status[7]",
            ),
            cv.Optional(CONF_STATUS_PACKET_CAPTURE, default={}): _with_default_name(
                text_sensor.text_sensor_schema(
                    icon="mdi:code-brackets",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                ),
                "Status Packet Capture",
            ),
            cv.Optional(CONF_HEAD, default={}): _with_default_name(
                cover.cover_schema(TemperBridgeNovaCover, icon="mdi:bed"),
                "Head",
            ),
            cv.Optional(CONF_LEGS, default={}): _with_default_name(
                cover.cover_schema(TemperBridgeNovaCover, icon="mdi:bed"),
                "Legs",
            ),
            cv.Optional(CONF_LUMBAR, default={}): _with_default_name(
                cover.cover_schema(TemperBridgeNovaCover, icon="mdi:bed"),
                "Lumbar",
            ),
            cv.Optional(CONF_HEAD_RAISE, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-up-bold"
                ),
                "Head Raise",
            ),
            cv.Optional(CONF_HEAD_LOWER, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-down-bold"
                ),
                "Head Lower",
            ),
            cv.Optional(CONF_LEGS_RAISE, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-up-bold"
                ),
                "Legs Raise",
            ),
            cv.Optional(CONF_LEGS_LOWER, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-down-bold"
                ),
                "Legs Lower",
            ),
            cv.Optional(CONF_LUMBAR_RAISE, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-up-bold"
                ),
                "Lumbar Raise",
            ),
            cv.Optional(CONF_LUMBAR_LOWER, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaActuatorButton, icon="mdi:arrow-down-bold"
                ),
                "Lumbar Lower",
            ),
            cv.Optional(CONF_STOP, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaStopButton, icon="mdi:stop"),
                "Stop",
            ),
            cv.Optional(CONF_FLAT, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:minus"),
                "Flat",
            ),
            cv.Optional(CONF_ZERO_G, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:angle-acute"),
                "Zero G",
            ),
            cv.Optional(CONF_TV, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:television"),
                "TV",
            ),
            cv.Optional(CONF_FAVORITE_1, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:numeric-1-box"),
                "Favorite 1",
            ),
            cv.Optional(CONF_FAVORITE_2, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:numeric-2-box"),
                "Favorite 2",
            ),
            cv.Optional(CONF_ANTI_SNORE, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:bed"),
                "Anti-Snore",
            ),
            cv.Optional(CONF_TOGGLE_LIGHTS, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:lightbulb"),
                "Toggle Lights",
            ),
            cv.Optional(CONF_MASSAGE_WAVE_MODE, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:waves"),
                "Massage Wave Mode",
            ),
            cv.Optional(CONF_HEAD_ZONE_MASSAGE, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:vibrate"),
                "Head Zone Massage",
            ),
            cv.Optional(CONF_FOOT_ZONE_MASSAGE, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:vibrate"),
                "Foot Zone Massage",
            ),
            cv.Optional(CONF_CAPTURE_STATUS_PACKETS, default={}): _with_default_name(
                button.button_schema(
                    TemperBridgeNovaStatusPacketCaptureButton,
                    icon="mdi:database-search",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                ),
                "Capture Status Packets",
            ),
            cv.Optional(CONF_LOG_STATUS_PACKETS, default={}): _with_default_name(
                switch.switch_schema(
                    TemperBridgeNovaStatusPacketLogSwitch,
                    icon="mdi:console-line",
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    default_restore_mode="ALWAYS_OFF",
                ),
                "Log Status Packets",
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "temperbridge_nova",
    baud_rate=38400,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    parity="EVEN",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    rx_enable_pin = await cg.gpio_pin_expression(config[CONF_RX_ENABLE_PIN])
    cg.add(var.set_rx_enable_pin(rx_enable_pin))

    for index, pin_config in enumerate(config[CONF_BOARD_ID_PINS]):
        board_id_pin = await cg.gpio_pin_expression(pin_config)
        cg.add(var.set_board_id_pin(index, board_id_pin))

    board_id_sensor = await sensor.new_sensor(config[CONF_BOARD_ID])
    cg.add(var.set_board_id_sensor(board_id_sensor))

    link_state_sensor = await text_sensor.new_text_sensor(config[CONF_LINK_STATE])
    cg.add(var.set_mfp_link_state_sensor(link_state_sensor))

    key_sensor = await text_sensor.new_text_sensor(config[CONF_KEY])
    cg.add(var.set_key_sensor(key_sensor))

    control_box_model_sensor = await text_sensor.new_text_sensor(config[CONF_CONTROL_BOX_MODEL])
    cg.add(var.set_control_box_model_sensor(control_box_model_sensor))

    movement_state_sensor = await text_sensor.new_text_sensor(config[CONF_MOVEMENT_STATE])
    cg.add(var.set_movement_state_sensor(movement_state_sensor))

    head_pulse_sensor = await sensor.new_sensor(config[CONF_HEAD_PULSE])
    cg.add(var.set_head_pulse_sensor(head_pulse_sensor))

    legs_pulse_sensor = await sensor.new_sensor(config[CONF_LEGS_PULSE])
    cg.add(var.set_legs_pulse_sensor(legs_pulse_sensor))

    lumbar_pulse_sensor = await sensor.new_sensor(config[CONF_LUMBAR_PULSE])
    cg.add(var.set_lumbar_pulse_sensor(lumbar_pulse_sensor))

    status_0_sensor = await sensor.new_sensor(config[CONF_STATUS_0])
    cg.add(var.set_status_0_sensor(status_0_sensor))

    status_7_sensor = await sensor.new_sensor(config[CONF_STATUS_7])
    cg.add(var.set_status_7_sensor(status_7_sensor))

    status_packet_capture_sensor = await text_sensor.new_text_sensor(config[CONF_STATUS_PACKET_CAPTURE])
    cg.add(var.set_status_packet_capture_sensor(status_packet_capture_sensor))

    head_cover = await cover.new_cover(config[CONF_HEAD])
    cg.add(head_cover.set_parent(var))
    cg.add(head_cover.set_actuator(MfpActuator.HEAD))
    cg.add(var.set_head_cover(head_cover))

    legs_cover = await cover.new_cover(config[CONF_LEGS])
    cg.add(legs_cover.set_parent(var))
    cg.add(legs_cover.set_actuator(MfpActuator.LEGS))
    cg.add(var.set_legs_cover(legs_cover))

    lumbar_cover = await cover.new_cover(config[CONF_LUMBAR])
    cg.add(lumbar_cover.set_parent(var))
    cg.add(lumbar_cover.set_actuator(MfpActuator.LUMBAR))
    cg.add(var.set_lumbar_cover(lumbar_cover))

    await _new_actuator_button(
        config, var, CONF_HEAD_RAISE, MfpActuator.HEAD, MfpActuatorDirection.RAISE
    )

    await _new_actuator_button(
        config, var, CONF_HEAD_LOWER, MfpActuator.HEAD, MfpActuatorDirection.LOWER
    )

    await _new_actuator_button(
        config, var, CONF_LEGS_RAISE, MfpActuator.LEGS, MfpActuatorDirection.RAISE
    )

    await _new_actuator_button(
        config, var, CONF_LEGS_LOWER, MfpActuator.LEGS, MfpActuatorDirection.LOWER
    )

    await _new_actuator_button(
        config, var, CONF_LUMBAR_RAISE, MfpActuator.LUMBAR, MfpActuatorDirection.RAISE
    )

    await _new_actuator_button(
        config, var, CONF_LUMBAR_LOWER, MfpActuator.LUMBAR, MfpActuatorDirection.LOWER
    )

    stop_button = await button.new_button(config[CONF_STOP])
    cg.add(stop_button.set_parent(var))

    await button.new_button(_with_send_key_action(config, CONF_FLAT))

    await button.new_button(_with_send_key_action(config, CONF_ZERO_G))

    await button.new_button(_with_send_key_action(config, CONF_TV))

    await button.new_button(_with_send_key_action(config, CONF_FAVORITE_1))

    await button.new_button(_with_send_key_action(config, CONF_FAVORITE_2))

    await button.new_button(_with_send_key_action(config, CONF_ANTI_SNORE))

    await button.new_button(_with_send_key_action(config, CONF_TOGGLE_LIGHTS))

    await button.new_button(_with_send_key_action(config, CONF_MASSAGE_WAVE_MODE))

    await button.new_button(_with_send_key_action(config, CONF_HEAD_ZONE_MASSAGE))

    await button.new_button(_with_send_key_action(config, CONF_FOOT_ZONE_MASSAGE))

    capture_status_packets_button = await button.new_button(config[CONF_CAPTURE_STATUS_PACKETS])
    cg.add(capture_status_packets_button.set_parent(var))

    log_status_packets_switch = await switch.new_switch(config[CONF_LOG_STATUS_PACKETS])
    cg.add(log_status_packets_switch.set_parent(var))


def validate_key_code(value):
    value = cv.string_strict(value)
    if len(value) != 8:
        raise cv.Invalid("MFP key code must be exactly 8 hex characters")
    for char in value:
        if char not in "0123456789abcdefABCDEF":
            raise cv.Invalid("MFP key code must contain only hex characters")
    return value


@automation.register_action(
    "temperbridge_nova.send_key",
    TemperBridgeNovaSendKeyAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(TemperBridgeNovaComponent),
            cv.Required(CONF_KEY_CODE): cv.templatable(validate_key_code),
        }
    ),
    synchronous=True,
)
async def send_key_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    key_code = await cg.templatable(config[CONF_KEY_CODE], args, cg.std_string)
    cg.add(var.set_key_code(key_code))
    return var
