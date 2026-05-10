#include "temperbridge_nova.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "status_packet_view.h"

namespace esphome {
namespace temperbridge_nova {

namespace {

static const char *const TAG = "temperbridge_nova";

uint8_t mfp_checksum(const uint8_t *data, size_t length) {
    uint8_t checksum = 0xFF;
    for (size_t i = 0; i < length; ++i) {
        checksum -= data[i];
    }
    return checksum;
}

bool is_status_packet(const uint8_t *data, size_t length) {
    return data != nullptr && length > 1 && data[1] == 0x07;
}

std::string format_packet_capture(size_t index, size_t total, const uint8_t *data, size_t length) {
    char prefix[16];
    std::snprintf(prefix, sizeof(prefix), "%u/%u: ", static_cast<unsigned>(index), static_cast<unsigned>(total));

    std::string payload;
    payload += prefix;

    for (size_t i = 0; i < length; ++i) {
        char byte_payload[sizeof("0x00")];
        std::snprintf(byte_payload, sizeof(byte_payload), "0x%02X", data[i]);
        payload += byte_payload;
        if (i + 1 < length) {
            payload += ' ';
        }
    }

    return payload;
}

}  // namespace

void TemperBridgeNovaComponent::process_uart_() {
    while (this->available() > 0) {
        if (this->_rx_size >= this->_rx_buffer.size()) {
            ESP_LOGW(TAG, "MFP RX buffer overflow; dropping buffered bytes");
            this->_rx_size = 0;
        }

        uint8_t byte = 0;
        if (!this->read_byte(&byte)) {
            break;
        }

        this->_rx_buffer[this->_rx_size++] = byte;
    }

    while (this->_rx_size >= 1) {
        const size_t packet_length = static_cast<size_t>(this->_rx_buffer[0]) + 3;
        if (packet_length > this->_rx_buffer.size() || packet_length < 3) {
            ESP_LOGW(TAG, "Invalid MFP packet length byte: 0x%02X", this->_rx_buffer[0]);
            this->remove_rx_bytes_(1);
            continue;
        }

        if (this->_rx_size < packet_length) {
            return;
        }

        const uint8_t rx_checksum = this->_rx_buffer[packet_length - 1];
        const uint8_t calculated_checksum = mfp_checksum(this->_rx_buffer.data(), packet_length - 1);
        if (rx_checksum != calculated_checksum) {
            ESP_LOGW(TAG, "MFP checksum mismatch: expected 0x%02X, got 0x%02X", calculated_checksum, rx_checksum);
            this->remove_rx_bytes_(1);
            continue;
        }

        // TODO: there are other types of packets we could encounter on the bus,
        //  for example, 0x1 commands from other MFP devices.
        if (is_status_packet(this->_rx_buffer.data(), packet_length)) {
            this->process_status_packet_(this->_rx_buffer.data(), packet_length);
        }

        // If we've got a command to send, do it now.
        this->write_next_command_();
        this->remove_rx_bytes_(packet_length);
    }
}

void TemperBridgeNovaComponent::process_status_packet_(const uint8_t *data, size_t length) {
    StatusPacketView packet{data, length};
    if (!packet.valid()) {
        ESP_LOGW(TAG, "Ignoring short MFP status packet with length %u", static_cast<unsigned>(length));
        return;
    }

    const uint32_t now = millis();
    this->_last_status_ms = now;
    this->set_link_state_(MfpLinkState::ONLINE);
    this->capture_status_packet_(data, length);

    const uint8_t status_0 = packet.status_0();
    const uint8_t status_7 = packet.status_7();
    this->_status_0.publish_state(status_0);
    this->_status_7.publish_state(status_7);

    const ControlBoxModel control_box_model = lookup_control_box_model(status_0, status_7);
    if (control_box_model != this->_control_box_model) {
        ESP_LOGI(TAG, "Control box model changed: %s -> %s", control_box_model_to_string(this->_control_box_model),
                 control_box_model_to_string(control_box_model));
        this->_control_box_model = control_box_model;
    }
    this->_control_box_model_sensor.publish_state(control_box_model_to_string(this->_control_box_model));

    // Detects when user presses a button on the real remote control(s)
    const uint32_t key = packet.key();
    this->publish_key_(key);
    if (key != 0) {
        ESP_LOGD(TAG, "MFP key: %08X", static_cast<unsigned>(key));
    }

    this->_head_pulse.update(packet.head_pulse(), now);
    this->_legs_pulse.update(packet.legs_pulse(), now);
    this->_lumbar_pulse.update(packet.lumbar_pulse(), now);
    this->process_hard_limit_detection_(now);
}

void TemperBridgeNovaComponent::process_status_timeout_() {
    if (this->_link_state != MfpLinkState::ONLINE || !this->_last_status_ms.has_value()) {
        return;
    }

    if (millis() - *this->_last_status_ms > STATUS_OFFLINE_TIMEOUT_MS) {
        this->_last_status_ms.reset();
        this->set_link_state_(MfpLinkState::OFFLINE);
    }
}

void TemperBridgeNovaComponent::enqueue_uart_command_(const MfpCommandBytes &command) {
    if (this->_command_queue.push(command, millis())) {
        ESP_LOGW(TAG, "MFP command queue full; dropped oldest command");
    }
}

void TemperBridgeNovaComponent::clear_uart_command_queue_() {
    this->_command_queue.clear();
}

void TemperBridgeNovaComponent::capture_status_packet_(const uint8_t *data, size_t length) {
    if (this->_status_packets_remaining == 0) {
        return;
    }

    ++this->_status_packet_capture_index;
    --this->_status_packets_remaining;

    if (this->_status_packet_capture_sensor != nullptr) {
        this->_status_packet_capture_sensor->publish_state(
            format_packet_capture(this->_status_packet_capture_index, STATUS_PACKET_CAPTURE_COUNT, data, length));
    }

    if (this->_status_packets_remaining == 0) {
        ESP_LOGI(TAG, "Finished MFP status packet capture");
    }
}

void TemperBridgeNovaComponent::write_next_command_() {
    while (const auto command = this->_command_queue.pop()) {
        const uint32_t age_ms = millis() - command->queued_at_ms;
        if (age_ms > MAX_QUEUED_COMMAND_AGE_MS) {
            ESP_LOGW(TAG, "Dropping stale MFP UART command queued %u ms ago", static_cast<unsigned>(age_ms));
            continue;
        }

        this->write_array(command->bytes.data(), command->bytes.size());
        this->flush();
        return;
    }
}

void TemperBridgeNovaComponent::remove_rx_bytes_(size_t count) {
    if (count >= this->_rx_size) {
        this->_rx_size = 0;
        return;
    }

    std::memmove(this->_rx_buffer.data(), this->_rx_buffer.data() + count, this->_rx_size - count);
    this->_rx_size -= count;
}

}  // namespace temperbridge_nova
}  // namespace esphome
