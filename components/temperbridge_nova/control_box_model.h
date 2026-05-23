#ifndef ESPHOME_TEMPERBRIDGE_NOVA_CONTROL_BOX_MODEL_H
#define ESPHOME_TEMPERBRIDGE_NOVA_CONTROL_BOX_MODEL_H

#include <cstdint>

namespace esphome {
namespace temperbridge_nova {

enum class ControlBoxModel : uint8_t {
    UNKNOWN = 0,
    MC232 = 1,
    CU358 = 2,
    MC120 = 3,
};

struct ControlBoxModelSignature {
    uint8_t status_0{0};
    uint8_t status_7{0};
    ControlBoxModel model{ControlBoxModel::UNKNOWN};
};

ControlBoxModel lookup_control_box_model(uint8_t status_0, uint8_t status_7);
const char *control_box_model_to_string(ControlBoxModel model);
bool control_box_model_supports_lumbar(ControlBoxModel model);

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_CONTROL_BOX_MODEL_H
