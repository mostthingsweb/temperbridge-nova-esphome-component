#include "automation.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace temperbridge_nova {

namespace {

static const char *const TAG = "temperbridge_nova.automation";

uint8_t mfp_command_checksum(const MfpCommandBytes &command) {
    uint8_t checksum = 0xFF;
    for (size_t i = 0; i + 1 < command.size(); ++i) {
        checksum -= command[i];
    }
    return checksum;
}

std::optional<MfpCommandBytes> parse_custom_key_code(const std::string &key_code) {
    if (key_code.size() != 8) {
        return {};
    }

    const auto parsed_key = parse_hex<uint32_t>(key_code);
    if (!parsed_key.has_value()) {
        return {};
    }

    const uint32_t key = *parsed_key;
    MfpCommandBytes command{0x05,
                            0x01,
                            static_cast<uint8_t>(key & 0xFF),
                            static_cast<uint8_t>((key >> 8) & 0xFF),
                            static_cast<uint8_t>((key >> 16) & 0xFF),
                            static_cast<uint8_t>((key >> 24) & 0xFF),
                            0x00,
                            0x00};
    command[7] = mfp_command_checksum(command);
    return command;
}

}  // namespace

void send_custom_key_action(TemperBridgeNovaComponent *parent, const std::string &key_code) {
    if (parent == nullptr) {
        return;
    }

    const auto command = parse_custom_key_code(key_code);
    if (!command.has_value()) {
        ESP_LOGW(TAG, "Ignoring custom MFP key command with invalid key code: %s", key_code.c_str());
        return;
    }

    parent->send_custom_key_code(*command, key_code);
}

}  // namespace temperbridge_nova
}  // namespace esphome
