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
    if (this->_parent == nullptr) {
        return;
    }

    if (call.get_stop()) {
        this->_parent->handle_stop_command();
        return;
    }

    const auto position = call.get_position();
    if (!position.has_value()) {
        return;
    }

    if (*position == cover::COVER_OPEN) {
        this->_parent->handle_cover_command(this->_actuator, true);
    } else if (*position == cover::COVER_CLOSED) {
        this->_parent->handle_cover_command(this->_actuator, false);
    }
}

void TemperBridgeNovaButton::press_action() {
    if (this->_parent == nullptr) {
        return;
    }
    this->_parent->handle_button_command(this->_command);
}

}  // namespace temperbridge_nova
}  // namespace esphome
