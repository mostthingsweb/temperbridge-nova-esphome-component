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

enum class MfpButtonCommand : uint8_t {
    STOP = 0,
    FLAT = 1,
    ZERO_G = 2,
    TV = 3,
    FAVORITE_1 = 4,
    FAVORITE_2 = 5,
    ANTI_SNORE = 6,
    TOGGLE_LIGHTS = 7,
    MASSAGE_WAVE_MODE = 8,
    HEAD_ZONE_MASSAGE = 9,
    FOOT_ZONE_MASSAGE = 10,
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

class TemperBridgeNovaButton : public button::Button, public Parented<TemperBridgeNovaComponent> {
public:
    void set_command(MfpButtonCommand command) {
        this->_command = command;
    }

protected:
    void press_action() override;

    MfpButtonCommand _command{MfpButtonCommand::STOP};
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
