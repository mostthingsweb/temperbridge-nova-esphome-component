#include "entities.h"

#include "temperbridge_nova.h"

namespace esphome {
namespace temperbridge_nova {

void TemperBridgeNovaCover::set_operation(cover::CoverOperation operation) {
    this->current_operation = operation;
    this->publish_state(false);
}

cover::CoverTraits TemperBridgeNovaCover::get_traits() {
    auto traits = cover::CoverTraits();
    traits.set_supports_stop(true);
    traits.set_is_assumed_state(true);
    return traits;
}

void TemperBridgeNovaCover::control(const cover::CoverCall &call) {
    if (this->parent_ == nullptr) {
        return;
    }

    if (call.get_stop()) {
        this->parent_->handle_stop_command();
        return;
    }

    const auto position = call.get_position();
    if (!position.has_value()) {
        return;
    }

    if (*position == cover::COVER_OPEN) {
        this->parent_->handle_cover_command(this->_actuator, true);
    } else if (*position == cover::COVER_CLOSED) {
        this->parent_->handle_cover_command(this->_actuator, false);
    }
}

void TemperBridgeNovaButton::press_action() {
    if (this->parent_ == nullptr) {
        return;
    }
    this->parent_->handle_button_command(this->_command);
}

void TemperBridgeNovaStatusPacketCaptureButton::press_action() {
    if (this->parent_ == nullptr) {
        return;
    }
    this->parent_->handle_status_packet_capture_command();
}

void TemperBridgeNovaStatusPacketLogSwitch::write_state(bool state) {
    if (this->parent_ == nullptr) {
        return;
    }
    this->parent_->set_status_packet_logging_enabled(state);
    this->publish_state(state);
}

}  // namespace temperbridge_nova
}  // namespace esphome
