#include "status_packet_view.h"

namespace esphome {
namespace temperbridge_nova {

namespace {

constexpr size_t STATUS_0_OFFSET = 0x00;
constexpr size_t STATUS_7_OFFSET = 0x07;
constexpr size_t KEY_OFFSET = 0x02;
constexpr size_t HEAD_PULSE_OFFSET = 0x12;
constexpr size_t LEGS_PULSE_OFFSET = 0x14;
constexpr size_t LUMBAR_PULSE_OFFSET = 0x16;
constexpr size_t LUMBAR_PULSE_HIGH_BYTE_OFFSET = 0x17;

}  // namespace

StatusPacketView::StatusPacketView(const uint8_t *data, size_t length) : _data(data), _length(length) {
}

bool StatusPacketView::valid() const {
    return this->_data != nullptr && this->_length > LUMBAR_PULSE_HIGH_BYTE_OFFSET;
}

uint8_t StatusPacketView::status_0() const {
    return this->_data[STATUS_0_OFFSET];
}

uint8_t StatusPacketView::status_7() const {
    return this->_data[STATUS_7_OFFSET];
}

uint32_t StatusPacketView::key() const {
    return this->u32_le(KEY_OFFSET);
}

uint16_t StatusPacketView::head_pulse() const {
    return this->u16_le(HEAD_PULSE_OFFSET);
}

uint16_t StatusPacketView::legs_pulse() const {
    return this->u16_le(LEGS_PULSE_OFFSET);
}

uint16_t StatusPacketView::lumbar_pulse() const {
    return this->u16_le(LUMBAR_PULSE_OFFSET);
}

uint16_t StatusPacketView::u16_le(size_t offset) const {
    return static_cast<uint16_t>(static_cast<uint16_t>(this->_data[offset]) |
                                 (static_cast<uint16_t>(this->_data[offset + 1]) << 8));
}

uint32_t StatusPacketView::u32_le(size_t offset) const {
    return static_cast<uint32_t>(this->_data[offset]) | (static_cast<uint32_t>(this->_data[offset + 1]) << 8) |
           (static_cast<uint32_t>(this->_data[offset + 2]) << 16) |
           (static_cast<uint32_t>(this->_data[offset + 3]) << 24);
}

}  // namespace temperbridge_nova
}  // namespace esphome
