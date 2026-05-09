#include "control_box_model.h"

#include <array>

namespace esphome {
namespace temperbridge_nova {

namespace {

constexpr std::array<ControlBoxModelSignature, 1> CONTROL_BOX_MODEL_SIGNATURES{{
    {37, 50, ControlBoxModel::MC232},
}};

}  // namespace

ControlBoxModel lookup_control_box_model(uint8_t status_0, uint8_t status_7) {
    for (const auto &signature : CONTROL_BOX_MODEL_SIGNATURES) {
        if (signature.status_0 == status_0 && signature.status_7 == status_7) {
            return signature.model;
        }
    }

    return ControlBoxModel::UNKNOWN;
}

const char *control_box_model_to_string(ControlBoxModel model) {
    switch (model) {
    case ControlBoxModel::MC232:
        return "MC232";
    case ControlBoxModel::UNKNOWN: /* fallthrough */
    default:
        return "unknown";
    }
}

}  // namespace temperbridge_nova
}  // namespace esphome
