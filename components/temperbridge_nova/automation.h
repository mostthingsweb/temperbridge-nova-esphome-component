#ifndef ESPHOME_TEMPERBRIDGE_NOVA_AUTOMATION_H
#define ESPHOME_TEMPERBRIDGE_NOVA_AUTOMATION_H

#include <string>

#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#include "temperbridge_nova.h"

namespace esphome {
namespace temperbridge_nova {

void send_custom_key_action(TemperBridgeNovaComponent *parent, const std::string &key_code);

template<typename... Ts> class TemperBridgeNovaSendKeyAction : public Action<Ts...>, public Parented<TemperBridgeNovaComponent> {
public:
    TEMPLATABLE_VALUE(std::string, key_code)

    void play(const Ts &...x) override {
        send_custom_key_action(this->parent_, this->key_code_.value(x...));
    }
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_AUTOMATION_H
