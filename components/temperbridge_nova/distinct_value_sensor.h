#ifndef ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H
#define ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H

#include <cstdint>
#include <optional>
#include <string>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace temperbridge_nova {

template<typename T> struct DistinctValueSensorTraits;

template<> struct DistinctValueSensorTraits<uint8_t> {
    using SensorType = sensor::Sensor;

    static void publish_state(SensorType *sensor, uint8_t value) {
        sensor->publish_state(value);
    }
};

template<> struct DistinctValueSensorTraits<float> {
    using SensorType = sensor::Sensor;

    static void publish_state(SensorType *sensor, float value) {
        sensor->publish_state(value);
    }
};

template<> struct DistinctValueSensorTraits<std::string> {
    using SensorType = text_sensor::TextSensor;

    static void publish_state(SensorType *sensor, const std::string &value) {
        sensor->publish_state(value);
    }
};

/// Small helper that only publishes to the sensor when the value is different.
template<typename T> class DistinctValueSensor {
public:
    using SensorType = typename DistinctValueSensorTraits<T>::SensorType;

    void set_sensor(SensorType *sensor) {
        this->_sensor = sensor;
    }

    SensorType *sensor() const {
        return this->_sensor;
    }

    void publish_state(const T &value) {
        if (this->_last_value.has_value() && *this->_last_value == value) {
            return;
        }

        this->_last_value = value;
        if (this->_sensor != nullptr) {
            DistinctValueSensorTraits<T>::publish_state(this->_sensor, value);
        }
    }

private:
    SensorType *_sensor{nullptr};
    std::optional<T> _last_value{};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H
