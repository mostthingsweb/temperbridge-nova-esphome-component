#include "distinct_value_sensor.h"

namespace esphome {
namespace temperbridge_nova {

void DistinctValueSensor::publish_state(float value) {
    if (this->_last_value.has_value() && *this->_last_value == value) {
        return;
    }

    this->_last_value = value;
    if (this->_sensor != nullptr) {
        this->_sensor->publish_state(value);
    }
}

}  // namespace temperbridge_nova
}  // namespace esphome
