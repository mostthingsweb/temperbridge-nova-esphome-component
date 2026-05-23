#include "temperbridge_nova.h"

#include <cstdint>
#include <cstdio>
#include <string>

#include "esphome/components/cover/cover.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "mfp_commands.h"

namespace esphome {
namespace temperbridge_nova {

namespace {

static const char *const TAG = "temperbridge_nova";

const char *link_state_to_string(MfpLinkState state) {
    switch (state) {
    case MfpLinkState::ONLINE:
        return "online";
    case MfpLinkState::OFFLINE:
        return "offline";
    case MfpLinkState::UNKNOWN: /* fallthrough */
    default:
        return "unknown";
    }
}

const char *movement_state_to_string(MovementState state) {
    switch (state) {
    case MovementState::HEAD_RAISING:
        return "head raising";
    case MovementState::HEAD_LOWERING:
        return "head lowering";
    case MovementState::LEGS_RAISING:
        return "legs raising";
    case MovementState::LEGS_LOWERING:
        return "legs lowering";
    case MovementState::LUMBAR_RAISING:
        return "lumbar raising";
    case MovementState::LUMBAR_LOWERING:
        return "lumbar lowering";
    case MovementState::STOPPED:
        return "stopped";
    }
    return "unknown";
}

cover::CoverOperation actuator_operation(MovementState state, MfpActuator actuator) {
    switch (actuator) {
    case MfpActuator::HEAD:
        if (state == MovementState::HEAD_RAISING) {
            return cover::COVER_OPERATION_OPENING;
        }
        if (state == MovementState::HEAD_LOWERING) {
            return cover::COVER_OPERATION_CLOSING;
        }
        break;
    case MfpActuator::LEGS:
        if (state == MovementState::LEGS_RAISING) {
            return cover::COVER_OPERATION_OPENING;
        }
        if (state == MovementState::LEGS_LOWERING) {
            return cover::COVER_OPERATION_CLOSING;
        }
        break;
    case MfpActuator::LUMBAR:
        if (state == MovementState::LUMBAR_RAISING) {
            return cover::COVER_OPERATION_OPENING;
        }
        if (state == MovementState::LUMBAR_LOWERING) {
            return cover::COVER_OPERATION_CLOSING;
        }
        break;
    }
    return cover::COVER_OPERATION_IDLE;
}

MovementState actuator_movement_state(MfpActuator actuator, bool open) {
    switch (actuator) {
    case MfpActuator::HEAD:
        return open ? MovementState::HEAD_RAISING : MovementState::HEAD_LOWERING;
    case MfpActuator::LEGS:
        return open ? MovementState::LEGS_RAISING : MovementState::LEGS_LOWERING;
    case MfpActuator::LUMBAR:
        return open ? MovementState::LUMBAR_RAISING : MovementState::LUMBAR_LOWERING;
    }
    return MovementState::STOPPED;
}

const char *actuator_command_name(MfpActuator actuator, bool open) {
    switch (actuator) {
    case MfpActuator::HEAD:
        return open ? "head up" : "head down";
    case MfpActuator::LEGS:
        return open ? "legs up" : "legs down";
    case MfpActuator::LUMBAR:
        return open ? "lumbar up" : "lumbar down";
    }
    return "unknown movement";
}

MovementState actuator_movement_state(MfpActuator actuator, MfpActuatorDirection direction) {
    return actuator_movement_state(actuator, direction == MfpActuatorDirection::RAISE);
}

const char *actuator_command_name(MfpActuator actuator, MfpActuatorDirection direction) {
    return actuator_command_name(actuator, direction == MfpActuatorDirection::RAISE);
}

}  // namespace

void TemperBridgeNovaComponent::setup() {
    this->check_uart_settings(38400, 1, uart::UART_CONFIG_PARITY_EVEN, 8);

    // Just set RX en pin as enabled
    if (this->_rx_enable_pin != nullptr) {
        this->_rx_enable_pin->setup();
        this->_rx_enable_pin->digital_write(false);
    }

    this->setup_board_id_();
    this->publish_link_state_(MfpLinkState::UNKNOWN);
    this->publish_key_(0);
    this->_control_box_model_sensor.publish_state(control_box_model_to_string(this->_control_box_model));
    this->publish_control_box_capabilities_();
    this->publish_movement_state_(MovementState::STOPPED);

    this->_next_movement_command_ms = millis();
}

void TemperBridgeNovaComponent::loop() {
    this->process_uart_();
    this->process_status_timeout_();
    this->process_momentary_command_timeout_();
    this->process_movement_timer_();
    const uint32_t now = millis();
    this->_head_pulse.publish_pending(now);
    this->_legs_pulse.publish_pending(now);
    this->_lumbar_pulse.publish_pending(now);
}

void TemperBridgeNovaComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "TemperBridge Nova");
    LOG_PIN("  RX Enable Pin: ", this->_rx_enable_pin);
    LOG_SENSOR("  ", "Board ID", this->_board_id_sensor);
    LOG_TEXT_SENSOR("  ", "MFP Link State", this->_mfp_link_state_sensor);
    LOG_TEXT_SENSOR("  ", "MFP Key", this->_key_sensor);
    LOG_TEXT_SENSOR("  ", "Control Box Model", this->_control_box_model_sensor.sensor());
    LOG_TEXT_SENSOR("  ", "Control Box Signature", this->_control_box_signature_sensor.sensor());
    LOG_BINARY_SENSOR("  ", "Lumbar Supported", this->_lumbar_supported_sensor);
    LOG_TEXT_SENSOR("  ", "Movement State", this->_movement_state_sensor.sensor());
    LOG_TEXT_SENSOR("  ", "Status Packet Capture", this->_status_packet_capture_sensor);
    LOG_SENSOR("  ", "Head Pulse", this->_head_pulse.sensor());
    LOG_SENSOR("  ", "Legs Pulse", this->_legs_pulse.sensor());
    LOG_SENSOR("  ", "Lumbar Pulse", this->_lumbar_pulse.sensor());
    LOG_COVER("  ", "Head", this->_head_cover);
    LOG_COVER("  ", "Legs", this->_legs_cover);
    LOG_COVER("  ", "Lumbar", this->_lumbar_cover);
}

float TemperBridgeNovaComponent::get_setup_priority() const {
    return setup_priority::HARDWARE;
}

void TemperBridgeNovaComponent::set_board_id_pin(size_t index, InternalGPIOPin *pin) {
    if (index >= this->_board_id_pins.size()) {
        return;
    }
    this->_board_id_pins[index] = pin;
}

void TemperBridgeNovaComponent::handle_cover_command(MfpActuator actuator, bool open) {
    if (!this->actuator_supported_(actuator)) {
        ESP_LOGW(TAG, "Ignoring %s command because actuator is not supported by control box model %s",
                 actuator_command_name(actuator, open), control_box_model_to_string(this->_control_box_model));
        return;
    }

    this->handle_movement_command_(actuator_movement_state(actuator, open), actuator_command_name(actuator, open));
}

void TemperBridgeNovaComponent::handle_momentary_actuator_command(MfpActuator actuator,
                                                                  MfpActuatorDirection direction) {
    const auto requested_state = actuator_movement_state(actuator, direction);
    const auto *command_name = actuator_command_name(actuator, direction);
    if (!this->actuator_supported_(actuator)) {
        ESP_LOGW(TAG, "Ignoring momentary %s command because actuator is not supported by control box model %s",
                 command_name, control_box_model_to_string(this->_control_box_model));
        return;
    }

    if (this->_link_state != MfpLinkState::ONLINE) {
        ESP_LOGW(TAG, "Ignoring momentary %s command while MFP link is %s", command_name,
                 link_state_to_string(this->_link_state));
        this->clear_momentary_command_();
        this->_movement_state = MovementState::STOPPED;
        this->reset_hard_limit_detection_();
        this->publish_movement_state_(MovementState::STOPPED);
        return;
    }

    const uint32_t now = millis();
    this->_momentary_command_deadline_ms = now + MOMENTARY_COMMAND_TIMEOUT_MS;
    if (this->_movement_state == requested_state) {
        return;
    }

    this->clear_uart_command_queue_();
    this->_movement_state = requested_state;
    this->publish_movement_state_(requested_state);
    this->_next_movement_command_ms = now;
    this->start_hard_limit_detection_(now);
}

void TemperBridgeNovaComponent::handle_stop_command() {
    this->handle_button_command_(mfp_commands::STOP, "movement stop");
}

void TemperBridgeNovaComponent::handle_stop_button_command() {
    this->handle_button_command_(mfp_commands::STOP, "stop");
}

void TemperBridgeNovaComponent::handle_status_packet_capture_command() {
    this->start_status_packet_capture_();
}

void TemperBridgeNovaComponent::send_custom_key_code(const MfpCommandBytes &command) {
    this->handle_button_command_(command, "custom key");
}

void TemperBridgeNovaComponent::set_status_packet_logging_enabled(bool enabled) {
    this->_status_packet_logging_enabled = enabled;
}

void TemperBridgeNovaComponent::setup_board_id_() {
    for (auto *pin : this->_board_id_pins) {
        if (pin != nullptr) {
            pin->setup();
        }
    }

    uint8_t bits = 0;
    for (size_t i = 0; i < this->_board_id_pins.size(); ++i) {
        auto *pin = this->_board_id_pins[i];
        if (pin != nullptr && pin->digital_read()) {
            bits |= static_cast<uint8_t>(1U << i);
        }
    }

    if (this->_board_id_sensor != nullptr) {
        this->_board_id_sensor->publish_state(bits);
    }
}

void TemperBridgeNovaComponent::process_movement_timer_() {
    const uint32_t now = millis();
    if (static_cast<int32_t>(now - this->_next_movement_command_ms) < 0) {
        return;
    }

    this->enqueue_current_movement_command_();
    this->_next_movement_command_ms = now + MOVEMENT_COMMAND_PERIOD_MS;
}

void TemperBridgeNovaComponent::process_momentary_command_timeout_() {
    if (!this->_momentary_command_deadline_ms.has_value()) {
        return;
    }

    const uint32_t now = millis();
    if (static_cast<int32_t>(now - *this->_momentary_command_deadline_ms) < 0) {
        return;
    }

    ESP_LOGI(TAG, "Momentary actuator command timed out; stopping movement commands");
    this->clear_momentary_command_();
    this->clear_uart_command_queue_();
    this->_movement_state = MovementState::STOPPED;
    this->reset_hard_limit_detection_();
    if (this->_link_state == MfpLinkState::ONLINE) {
        this->enqueue_uart_command_(mfp_commands::STOP);
    }
    this->publish_movement_state_(MovementState::STOPPED);
}

bool TemperBridgeNovaComponent::actuator_supported_(MfpActuator actuator) const {
    switch (actuator) {
    case MfpActuator::HEAD:
    case MfpActuator::LEGS:
        return true;
    case MfpActuator::LUMBAR:
        return control_box_model_supports_lumbar(this->_control_box_model);
    }
    return false;
}

void TemperBridgeNovaComponent::process_hard_limit_detection_(uint32_t now_ms) {
    if (this->_movement_state == MovementState::STOPPED) {
        this->reset_hard_limit_detection_();
        return;
    }

    if (!this->_movement_started_ms.has_value()) {
        this->_movement_started_ms = now_ms;
    }

    if (now_ms - *this->_movement_started_ms < HARD_LIMIT_STARTUP_GRACE_MS) {
        return;
    }

    const auto *active_pulse = this->active_pulse_sensor_();
    if (active_pulse == nullptr || !active_pulse->velocity().has_value()) {
        return;
    }

    const float velocity = *active_pulse->velocity();
    const float abs_velocity = velocity < 0.0f ? -velocity : velocity;
    if (abs_velocity > HARD_LIMIT_MAX_ABS_VELOCITY) {
        this->_stationary_since_ms.reset();
        return;
    }

    if (!this->_stationary_since_ms.has_value()) {
        this->_stationary_since_ms = now_ms;
        return;
    }

    if (now_ms - *this->_stationary_since_ms < HARD_LIMIT_STATIONARY_MS) {
        return;
    }

    ESP_LOGI(TAG, "Detected hard limit while %s; stopping movement commands (velocity %.1f pulses/s)",
             movement_state_to_string(this->_movement_state), velocity);
    this->clear_uart_command_queue_();
    this->clear_momentary_command_();
    this->_movement_state = MovementState::STOPPED;
    this->reset_hard_limit_detection_();
    this->publish_movement_state_(MovementState::STOPPED);
}

void TemperBridgeNovaComponent::publish_link_state_(MfpLinkState state) {
    if (this->_mfp_link_state_sensor != nullptr) {
        this->_mfp_link_state_sensor->publish_state(link_state_to_string(state));
    }
}

void TemperBridgeNovaComponent::publish_key_(uint32_t key) {
    if (key != 0) {
        this->clear_uart_command_queue_();
        this->clear_momentary_command_();
        this->reset_hard_limit_detection_();
        if (this->_movement_state != MovementState::STOPPED) {
            ESP_LOGI(TAG, "MFP key press detected; stopping local movement commands");
            this->_movement_state = MovementState::STOPPED;
            this->publish_movement_state_(MovementState::STOPPED);
        }
    }

    if (this->_last_key.has_value() && *this->_last_key == key) {
        return;
    }

    this->_last_key = key;

    if (this->_key_sensor == nullptr) {
        return;
    }

    if (key == 0) {
        this->_key_sensor->publish_state("none");
        return;
    }

    char key_payload[9];
    std::snprintf(key_payload, sizeof(key_payload), "%08X", static_cast<unsigned>(key));
    this->_key_sensor->publish_state(key_payload);
}

void TemperBridgeNovaComponent::publish_control_box_capabilities_() {
    if (this->_lumbar_supported_sensor != nullptr) {
        this->_lumbar_supported_sensor->publish_state(control_box_model_supports_lumbar(this->_control_box_model));
    }
}

void TemperBridgeNovaComponent::set_link_state_(MfpLinkState state) {
    if (this->_link_state == state) {
        return;
    }

    const MfpLinkState old_state = this->_link_state;
    this->_link_state = state;
    ESP_LOGI(TAG, "MFP link state changed: %s -> %s", link_state_to_string(old_state), link_state_to_string(state));

    if (old_state == MfpLinkState::ONLINE || state == MfpLinkState::ONLINE) {
        this->clear_uart_command_queue_();
    }

    this->publish_link_state_(state);
    if (state != MfpLinkState::ONLINE) {
        this->clear_momentary_command_();
        this->_movement_state = MovementState::STOPPED;
        this->reset_hard_limit_detection_();
        this->publish_key_(0);
        this->publish_movement_state_(MovementState::STOPPED);
    }
}

void TemperBridgeNovaComponent::publish_movement_state_(MovementState state) {
    this->_movement_state_sensor.publish_state(movement_state_to_string(state));

    if (this->_head_cover != nullptr) {
        this->_head_cover->set_operation(actuator_operation(state, MfpActuator::HEAD));
    }
    if (this->_legs_cover != nullptr) {
        this->_legs_cover->set_operation(actuator_operation(state, MfpActuator::LEGS));
    }
    if (this->_lumbar_cover != nullptr) {
        this->_lumbar_cover->set_operation(actuator_operation(state, MfpActuator::LUMBAR));
    }
}

void TemperBridgeNovaComponent::handle_movement_command_(MovementState requested_state, const char *command_name) {
    this->clear_momentary_command_();
    if (this->_link_state != MfpLinkState::ONLINE) {
        ESP_LOGW(TAG, "Ignoring %s command while MFP link is %s", command_name,
                 link_state_to_string(this->_link_state));
        this->_movement_state = MovementState::STOPPED;
        this->reset_hard_limit_detection_();
        this->publish_movement_state_(MovementState::STOPPED);
        return;
    }

    const uint32_t now = millis();
    this->clear_uart_command_queue_();
    this->_movement_state = requested_state;
    this->publish_movement_state_(requested_state);
    this->_next_movement_command_ms = now;
    this->start_hard_limit_detection_(now);
}

void TemperBridgeNovaComponent::handle_button_command_(const MfpCommandBytes &command, const char *command_name) {
    ESP_LOGI(TAG, "Got the %s command", command_name);
    this->clear_uart_command_queue_();
    this->clear_momentary_command_();
    this->_movement_state = MovementState::STOPPED;
    this->reset_hard_limit_detection_();
    if (this->_link_state == MfpLinkState::ONLINE) {
        this->enqueue_uart_command_(command);
    }
    this->publish_movement_state_(MovementState::STOPPED);
}

void TemperBridgeNovaComponent::start_status_packet_capture_() {
    this->_status_packets_remaining = STATUS_PACKET_CAPTURE_COUNT;
    this->_status_packet_capture_index = 0;

    ESP_LOGI(TAG, "Capturing next %u MFP status packets", static_cast<unsigned>(STATUS_PACKET_CAPTURE_COUNT));
    if (this->_status_packet_capture_sensor != nullptr) {
        this->_status_packet_capture_sensor->publish_state("waiting for status packets");
    }
}

void TemperBridgeNovaComponent::enqueue_current_movement_command_() {
    if (this->_link_state != MfpLinkState::ONLINE) {
        return;
    }

    switch (this->_movement_state) {
    case MovementState::HEAD_RAISING:
        this->enqueue_uart_command_(mfp_commands::HEAD_UP);
        break;
    case MovementState::HEAD_LOWERING:
        this->enqueue_uart_command_(mfp_commands::HEAD_DOWN);
        break;
    case MovementState::LEGS_RAISING:
        this->enqueue_uart_command_(mfp_commands::LEGS_UP);
        break;
    case MovementState::LEGS_LOWERING:
        this->enqueue_uart_command_(mfp_commands::LEGS_DOWN);
        break;
    case MovementState::LUMBAR_RAISING:
        this->enqueue_uart_command_(mfp_commands::LUMBAR_UP);
        break;
    case MovementState::LUMBAR_LOWERING:
        this->enqueue_uart_command_(mfp_commands::LUMBAR_DOWN);
        break;
    case MovementState::STOPPED:
        break;
    }
}

void TemperBridgeNovaComponent::clear_momentary_command_() {
    this->_momentary_command_deadline_ms.reset();
}

void TemperBridgeNovaComponent::start_hard_limit_detection_(uint32_t now_ms) {
    this->_movement_started_ms = now_ms;
    this->_stationary_since_ms.reset();
}

void TemperBridgeNovaComponent::reset_hard_limit_detection_() {
    this->_movement_started_ms.reset();
    this->_stationary_since_ms.reset();
}

const ThrottledPulseSensor *TemperBridgeNovaComponent::active_pulse_sensor_() const {
    switch (this->_movement_state) {
    case MovementState::HEAD_RAISING:
    case MovementState::HEAD_LOWERING:
        return &this->_head_pulse;
    case MovementState::LEGS_RAISING:
    case MovementState::LEGS_LOWERING:
        return &this->_legs_pulse;
    case MovementState::LUMBAR_RAISING:
    case MovementState::LUMBAR_LOWERING:
        return &this->_lumbar_pulse;
    case MovementState::STOPPED:
        return nullptr;
    }
    return nullptr;
}

}  // namespace temperbridge_nova
}  // namespace esphome
