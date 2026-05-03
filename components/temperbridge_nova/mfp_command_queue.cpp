#include "mfp_command_queue.h"

namespace esphome {
namespace temperbridge_nova {

bool MfpCommandQueue::push(const MfpCommandBytes &command, uint32_t queued_at_ms) {
    const bool dropped_oldest = this->_commands.size() >= MAX_LENGTH;
    if (dropped_oldest) {
        this->_commands.pop_front();
    }

    this->_commands.push_back(QueuedMfpCommand{queued_at_ms, command});
    return dropped_oldest;
}

void MfpCommandQueue::clear() {
    this->_commands.clear();
}

std::optional<QueuedMfpCommand> MfpCommandQueue::pop() {
    if (this->_commands.empty()) {
        return std::nullopt;
    }

    QueuedMfpCommand command = this->_commands.front();
    this->_commands.pop_front();
    return command;
}

}  // namespace temperbridge_nova
}  // namespace esphome
