#ifndef ESPHOME_TEMPERBRIDGE_NOVA_STATUS_PACKET_VIEW_H
#define ESPHOME_TEMPERBRIDGE_NOVA_STATUS_PACKET_VIEW_H

#include <cstddef>
#include <cstdint>

namespace esphome {
namespace temperbridge_nova {

class StatusPacketView {
public:
    StatusPacketView(const uint8_t *data, size_t length);

    bool valid() const;
    uint32_t key() const;
    uint16_t head_pulse() const;
    uint16_t legs_pulse() const;
    uint16_t lumbar_pulse() const;

private:
    uint16_t u16_le(size_t offset) const;
    uint32_t u32_le(size_t offset) const;

    const uint8_t *_data{nullptr};
    size_t _length{0};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_STATUS_PACKET_VIEW_H
