#ifndef ESPHOME_TEMPERBRIDGE_NOVA_MFP_COMMAND_QUEUE_H
#define ESPHOME_TEMPERBRIDGE_NOVA_MFP_COMMAND_QUEUE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>

namespace esphome {
namespace temperbridge_nova {

// An 0x1 multi-function port (MFP) command
using MfpCommandBytes = std::array<uint8_t, 8>;

struct QueuedMfpCommand {
    uint32_t queued_at_ms{0};
    MfpCommandBytes bytes{};
};

class MfpCommandQueue {
public:
    bool push(const MfpCommandBytes &command, uint32_t queued_at_ms);
    void clear();
    std::optional<QueuedMfpCommand> pop();

private:
    static constexpr size_t MAX_LENGTH = 5;

    std::deque<QueuedMfpCommand> _commands{};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_MFP_COMMAND_QUEUE_H
