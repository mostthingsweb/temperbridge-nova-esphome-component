from esphome import automation
import esphome.codegen as cg
from esphome.components import button, cover, sensor, switch, text_sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_KEY,
    CONF_ID,
    CONF_NAME,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_PULSE,
    STATE_CLASS_MEASUREMENT,
)
from esphome import pins

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["button", "cover", "sensor", "switch", "text_sensor"]

CONF_BOARD_ID = "board_id"
CONF_BOARD_ID_PINS = "board_id_pins"
CONF_CAPTURE_STATUS_PACKETS = "capture_status_packets"
CONF_CONTROL_BOX_MODEL = "control_box_model"
CONF_FAVORITE_1 = "favorite_1"
CONF_FAVORITE_2 = "favorite_2"
CONF_FLAT = "flat"
CONF_KEY_CODE = "key_code"
CONF_HEAD = "head"
CONF_HEAD_PULSE = "head_pulse"
CONF_LEGS = "legs"
CONF_LEGS_PULSE = "legs_pulse"
CONF_LINK_STATE = "link_state"
CONF_LOG_STATUS_PACKETS = "log_status_packets"
CONF_LUMBAR = "lumbar"
CONF_LUMBAR_PULSE = "lumbar_pulse"
CONF_RX_ENABLE_PIN = "rx_enable_pin"
CONF_STOP = "stop"
CONF_STATUS_0 = "status_0"
CONF_STATUS_7 = "status_7"
CONF_STATUS_PACKET_CAPTURE = "status_packet_capture"
CONF_TV = "tv"
CONF_ZERO_G = "zero_g"

DEFAULT_BOARD_ID_PINS = [4, 5, 6, 7]
DEFAULT_RX_ENABLE_PIN = 10

temperbridge_nova_ns = cg.esphome_ns.namespace("temperbridge_nova")
TemperBridgeNovaComponent = temperbridge_nova_ns.class_(
    "TemperBridgeNovaComponent", cg.Component, uart.UARTDevice
)
TemperBridgeNovaCover = temperbridge_nova_ns.class_("TemperBridgeNovaCover", cover.Cover)
TemperBridgeNovaButton = temperbridge_nova_ns.class_("TemperBridgeNovaButton", button.Button)
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
            cv.Optional(CONF_STOP, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:stop"),
                "Stop",
            ),
            cv.Optional(CONF_FLAT, default={}): _with_default_name(
                button.button_schema(TemperBridgeNovaButton, icon="mdi:bed"),
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
    cg.add(head_cover.set_actuator(0))
    cg.add(var.set_head_cover(head_cover))

    legs_cover = await cover.new_cover(config[CONF_LEGS])
    cg.add(legs_cover.set_parent(var))
    cg.add(legs_cover.set_actuator(1))
    cg.add(var.set_legs_cover(legs_cover))

    lumbar_cover = await cover.new_cover(config[CONF_LUMBAR])
    cg.add(lumbar_cover.set_parent(var))
    cg.add(lumbar_cover.set_actuator(2))
    cg.add(var.set_lumbar_cover(lumbar_cover))

    stop_button = await button.new_button(config[CONF_STOP])
    cg.add(stop_button.set_parent(var))
    cg.add(stop_button.set_command(0))

    flat_button = await button.new_button(config[CONF_FLAT])
    cg.add(flat_button.set_parent(var))
    cg.add(flat_button.set_command(1))

    zero_g_button = await button.new_button(config[CONF_ZERO_G])
    cg.add(zero_g_button.set_parent(var))
    cg.add(zero_g_button.set_command(2))

    tv_button = await button.new_button(config[CONF_TV])
    cg.add(tv_button.set_parent(var))
    cg.add(tv_button.set_command(3))

    favorite_1_button = await button.new_button(config[CONF_FAVORITE_1])
    cg.add(favorite_1_button.set_parent(var))
    cg.add(favorite_1_button.set_command(4))

    favorite_2_button = await button.new_button(config[CONF_FAVORITE_2])
    cg.add(favorite_2_button.set_parent(var))
    cg.add(favorite_2_button.set_command(5))

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
