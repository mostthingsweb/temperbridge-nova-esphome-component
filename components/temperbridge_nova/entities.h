#ifndef ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H
#define ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H

#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace temperbridge_nova {

class TemperBridgeNovaComponent;

enum class MfpActuator : uint8_t {
    HEAD = 0,
    LEGS = 1,
    LUMBAR = 2,
};

enum class MfpActuatorDirection : uint8_t {
    RAISE = 0,
    LOWER = 1,
};

class TemperBridgeNovaCover : public cover::Cover, public Parented<TemperBridgeNovaComponent> {
public:
    void set_actuator(MfpActuator actuator) {
        this->_actuator = actuator;
    }

    void set_operation(cover::CoverOperation operation);

    cover::CoverTraits get_traits() override;

protected:
    void control(const cover::CoverCall &call) override;

    MfpActuator _actuator{MfpActuator::HEAD};
};

class TemperBridgeNovaButton : public button::Button {
protected:
    void press_action() override;
};

class TemperBridgeNovaStopButton : public button::Button, public Parented<TemperBridgeNovaComponent> {
protected:
    void press_action() override;
};

class TemperBridgeNovaActuatorButton : public button::Button, public Parented<TemperBridgeNovaComponent> {
public:
    void set_actuator(MfpActuator actuator) {
        this->_actuator = actuator;
    }

    void set_direction(MfpActuatorDirection direction) {
        this->_direction = direction;
    }

protected:
    void press_action() override;

    MfpActuator _actuator{MfpActuator::HEAD};
    MfpActuatorDirection _direction{MfpActuatorDirection::RAISE};
};

class TemperBridgeNovaStatusPacketCaptureButton : public button::Button, public Parented<TemperBridgeNovaComponent> {
protected:
    void press_action() override;
};

class TemperBridgeNovaStatusPacketLogSwitch : public switch_::Switch, public Parented<TemperBridgeNovaComponent> {
protected:
    void write_state(bool state) override;
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_ENTITIES_H
