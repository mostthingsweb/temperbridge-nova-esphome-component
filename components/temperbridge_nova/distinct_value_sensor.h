#ifndef ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H
#define ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H

#include <optional>

#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace temperbridge_nova {

/// Small helper that only publishes to the sensor when the value is different
class DistinctValueSensor {
public:
    void set_sensor(sensor::Sensor *sensor) {
        this->_sensor = sensor;
    }
    sensor::Sensor *sensor() const {
        return this->_sensor;
    }

    void publish_state(float value);

private:
    sensor::Sensor *_sensor{nullptr};
    std::optional<float> _last_value{};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_DISTINCT_VALUE_SENSOR_H
