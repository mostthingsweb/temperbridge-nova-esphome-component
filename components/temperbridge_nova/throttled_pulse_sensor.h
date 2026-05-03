#ifndef ESPHOME_TEMPERBRIDGE_NOVA_THROTTLED_PULSE_SENSOR_H
#define ESPHOME_TEMPERBRIDGE_NOVA_THROTTLED_PULSE_SENSOR_H

#include <cstdint>
#include <optional>

#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace temperbridge_nova {

class ThrottledPulseSensor {
public:
    explicit ThrottledPulseSensor(uint32_t min_publish_interval_ms);

    void set_sensor(sensor::Sensor *sensor) {
        this->_sensor = sensor;
    }
    sensor::Sensor *sensor() const {
        return this->_sensor;
    }

    void update(uint16_t value, uint32_t now_ms);
    void publish_pending(uint32_t now_ms);
    std::optional<float> velocity() const {
        return this->_velocity;
    }

private:
    static constexpr uint32_t MAX_VELOCITY_SAMPLE_INTERVAL_MS = 1000;

    struct RawSample {
        uint16_t value{0};
        uint32_t update_ms{0};
    };

    sensor::Sensor *_sensor{nullptr};
    std::optional<RawSample> _last_raw_sample{};
    std::optional<float> _velocity{};
    std::optional<uint16_t> _last_published_value{};
    std::optional<uint16_t> _pending_value{};
    std::optional<uint32_t> _last_publish_ms{};
    uint32_t _min_publish_interval_ms{0};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_THROTTLED_PULSE_SENSOR_H
