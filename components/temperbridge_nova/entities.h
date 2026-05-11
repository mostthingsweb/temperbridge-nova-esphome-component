#ifndef ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H
#define ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H

#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace temperbridge_nova {

class TemperBridgeNovaComponent;

enum class MfpActuator : uint8_t {
    HEAD = 0,
    LEGS = 1,
    LUMBAR = 2,
};

enum class MfpButtonCommand : uint8_t {
    STOP = 0,
    FLAT = 1,
    ZERO_G = 2,
    TV = 3,
    FAVORITE_1 = 4,
    FAVORITE_2 = 5,
};

class TemperBridgeNovaCover : public cover::Cover {
public:
    void set_parent(TemperBridgeNovaComponent *parent) {
        this->_parent = parent;
    }

    void set_actuator(uint8_t actuator) {
        this->_actuator = static_cast<MfpActuator>(actuator);
    }

    void set_operation(cover::CoverOperation operation);

    cover::CoverTraits get_traits() override;

protected:
    void control(const cover::CoverCall &call) override;

    TemperBridgeNovaComponent *_parent{nullptr};
    MfpActuator _actuator{MfpActuator::HEAD};
};

class TemperBridgeNovaButton : public button::Button {
public:
    void set_parent(TemperBridgeNovaComponent *parent) {
        this->_parent = parent;
    }

    void set_command(uint8_t command) {
        this->_command = static_cast<MfpButtonCommand>(command);
    }

protected:
    void press_action() override;

    TemperBridgeNovaComponent *_parent{nullptr};
    MfpButtonCommand _command{MfpButtonCommand::STOP};
};

class TemperBridgeNovaStatusPacketCaptureButton : public button::Button {
public:
    void set_parent(TemperBridgeNovaComponent *parent) {
        this->_parent = parent;
    }

protected:
    void press_action() override;

    TemperBridgeNovaComponent *_parent{nullptr};
};

class TemperBridgeNovaStatusPacketLogSwitch : public switch_::Switch {
public:
    void set_parent(TemperBridgeNovaComponent *parent) {
        this->_parent = parent;
    }

protected:
    void write_state(bool state) override;

    TemperBridgeNovaComponent *_parent{nullptr};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H
