#include "throttled_pulse_sensor.h"

namespace esphome {
namespace temperbridge_nova {

ThrottledPulseSensor::ThrottledPulseSensor(uint32_t min_publish_interval_ms) {
    this->_min_publish_interval_ms = min_publish_interval_ms;
}

void ThrottledPulseSensor::update(uint16_t value, uint32_t now_ms) {
    if (this->_last_raw_sample.has_value()) {
        const uint32_t elapsed_ms = now_ms - this->_last_raw_sample->update_ms;
        // Don't try to calculate the velocity if readings too far apart, e.g. due to UART glitch or something
        if (elapsed_ms > MAX_VELOCITY_SAMPLE_INTERVAL_MS) {
            this->_velocity.reset();
            this->_last_raw_sample = RawSample{value, now_ms};
        } else if (elapsed_ms > 0) {
            const int32_t delta = static_cast<int32_t>(value) - static_cast<int32_t>(this->_last_raw_sample->value);
            this->_velocity = static_cast<float>(delta) * 1000.0f / static_cast<float>(elapsed_ms);
            this->_last_raw_sample = RawSample{value, now_ms};
        }
    } else {
        this->_last_raw_sample = RawSample{value, now_ms};
    }

    if (this->_last_published_value.has_value() && *this->_last_published_value == value) {
        this->_pending_value.reset();
        return;
    }

    this->_pending_value = value;
    this->publish_pending(now_ms);
}

void ThrottledPulseSensor::publish_pending(uint32_t now_ms) {
    if (this->_sensor == nullptr || !this->_pending_value.has_value()) {
        return;
    }

    if (this->_last_publish_ms.has_value() && now_ms - *this->_last_publish_ms < this->_min_publish_interval_ms) {
        return;
    }

    this->_last_publish_ms = now_ms;
    this->_last_published_value = this->_pending_value;
    this->_pending_value.reset();
    this->_sensor->publish_state(*this->_last_published_value);
}

}  // namespace temperbridge_nova
}  // namespace esphome
