#ifndef ESPHOME_TEMPERBRIDGE_NOVA_TEMPERBRIDGE_NOVA_H
#define ESPHOME_TEMPERBRIDGE_NOVA_TEMPERBRIDGE_NOVA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"

#include "control_box_model.h"
#include "distinct_value_sensor.h"
#include "entities.h"
#include "mfp_command_queue.h"
#include "throttled_pulse_sensor.h"

namespace esphome {
namespace temperbridge_nova {

enum class MfpLinkState : uint8_t {
    UNKNOWN = 0,
    ONLINE = 1,
    OFFLINE = 2,
};

enum class MovementState : uint8_t {
    STOPPED = 0,
    HEAD_RAISING = 1,
    HEAD_LOWERING = 2,
    LEGS_RAISING = 3,
    LEGS_LOWERING = 4,
    LUMBAR_RAISING = 5,
    LUMBAR_LOWERING = 6,
};

class TemperBridgeNovaComponent : public Component, public uart::UARTDevice {
public:
    void setup() override;
    void loop() override;
    void dump_config() override;
    float get_setup_priority() const override;

    void set_rx_enable_pin(InternalGPIOPin *pin) {
        this->_rx_enable_pin = pin;
    }

    void set_board_id_pin(size_t index, InternalGPIOPin *pin);

    void set_board_id_sensor(sensor::Sensor *sensor) {
        this->_board_id_sensor = sensor;
    }

    void set_mfp_link_state_sensor(text_sensor::TextSensor *sensor) {
        this->_mfp_link_state_sensor = sensor;
    }

    void set_key_sensor(text_sensor::TextSensor *sensor) {
        this->_key_sensor = sensor;
    }

    void set_control_box_model_sensor(text_sensor::TextSensor *sensor) {
        this->_control_box_model_sensor.set_sensor(sensor);
    }

    void set_movement_state_sensor(text_sensor::TextSensor *sensor) {
        this->_movement_state_sensor.set_sensor(sensor);
    }

    void set_status_packet_capture_sensor(text_sensor::TextSensor *sensor) {
        this->_status_packet_capture_sensor = sensor;
    }

    void set_head_pulse_sensor(sensor::Sensor *sensor) {
        this->_head_pulse.set_sensor(sensor);
    }

    void set_legs_pulse_sensor(sensor::Sensor *sensor) {
        this->_legs_pulse.set_sensor(sensor);
    }

    void set_lumbar_pulse_sensor(sensor::Sensor *sensor) {
        this->_lumbar_pulse.set_sensor(sensor);
    }

    void set_status_0_sensor(sensor::Sensor *sensor) {
        this->_status_0.set_sensor(sensor);
    }

    void set_status_7_sensor(sensor::Sensor *sensor) {
        this->_status_7.set_sensor(sensor);
    }

    void set_head_cover(TemperBridgeNovaCover *cover) {
        this->_head_cover = cover;
    }

    void set_legs_cover(TemperBridgeNovaCover *cover) {
        this->_legs_cover = cover;
    }

    void set_lumbar_cover(TemperBridgeNovaCover *cover) {
        this->_lumbar_cover = cover;
    }

    void handle_cover_command(MfpActuator actuator, bool open);
    void handle_momentary_actuator_command(MfpActuator actuator, MfpActuatorDirection direction);
    void handle_stop_command();
    void handle_stop_button_command();
    void handle_status_packet_capture_command();
    void send_custom_key_code(const MfpCommandBytes &command);
    void set_status_packet_logging_enabled(bool enabled);

protected:
    /* Board ID */
    void setup_board_id_();
    std::array<InternalGPIOPin *, 4> _board_id_pins{{nullptr, nullptr, nullptr, nullptr}};
    sensor::Sensor *_board_id_sensor{nullptr};

    /* UART */
    // TODO: this is probably way more than it needs to be
    static constexpr size_t RX_BUFFER_SIZE = 128;
    static constexpr uint32_t STATUS_OFFLINE_TIMEOUT_MS = 2000;
    static constexpr uint32_t MAX_QUEUED_COMMAND_AGE_MS = 250;
    void process_uart_();
    void process_status_packet_(const uint8_t *data, size_t length);
    void process_status_timeout_();
    /// Enqueue command onto our internal queue; sends at earliest convienence
    void enqueue_uart_command_(const MfpCommandBytes &command);
    void clear_uart_command_queue_();
    void write_next_command_();
    void remove_rx_bytes_(size_t count);
    InternalGPIOPin *_rx_enable_pin{nullptr};
    std::array<uint8_t, RX_BUFFER_SIZE> _rx_buffer{};
    size_t _rx_size{0};
    MfpCommandQueue _command_queue{};

    /* Link and status publishing */
    void set_link_state_(MfpLinkState state);
    void publish_link_state_(MfpLinkState state);
    void publish_key_(uint32_t key);
    void publish_movement_state_(MovementState state);
    text_sensor::TextSensor *_mfp_link_state_sensor{nullptr};
    text_sensor::TextSensor *_key_sensor{nullptr};
    DistinctValueSensor<std::string> _control_box_model_sensor{};
    DistinctValueSensor<std::string> _movement_state_sensor{};
    DistinctValueSensor<uint8_t> _status_0{};
    DistinctValueSensor<uint8_t> _status_7{};
    static constexpr size_t STATUS_PACKET_CAPTURE_COUNT = 10;
    void start_status_packet_capture_();
    void capture_status_packet_(const uint8_t *data, size_t length);
    void log_status_packet_(const uint8_t *data, size_t length);
    text_sensor::TextSensor *_status_packet_capture_sensor{nullptr};
    bool _status_packet_logging_enabled{false};
    std::array<uint8_t, RX_BUFFER_SIZE> _last_logged_status_packet{};
    std::optional<size_t> _last_logged_status_packet_length{};
    size_t _status_packets_remaining{0};
    size_t _status_packet_capture_index{0};
    std::optional<uint32_t> _last_status_ms{};
    std::optional<uint32_t> _last_key{};
    ControlBoxModel _control_box_model{ControlBoxModel::UNKNOWN};
    MfpLinkState _link_state{MfpLinkState::UNKNOWN};

    /* Movement commands */
    static constexpr uint32_t MOVEMENT_COMMAND_PERIOD_MS = 500;
    static constexpr uint32_t MOMENTARY_COMMAND_TIMEOUT_MS = 900;
    static constexpr uint32_t PULSE_PUBLISH_MIN_INTERVAL_MS = 100;
    void process_momentary_command_timeout_();
    void process_movement_timer_();
    void handle_movement_command_(MovementState requested_state, const char *command_name);
    void handle_button_command_(const MfpCommandBytes &command, const char *command_name);
    void enqueue_current_movement_command_();
    void clear_momentary_command_();
    TemperBridgeNovaCover *_head_cover{nullptr};
    TemperBridgeNovaCover *_legs_cover{nullptr};
    TemperBridgeNovaCover *_lumbar_cover{nullptr};
    ThrottledPulseSensor _head_pulse{PULSE_PUBLISH_MIN_INTERVAL_MS};
    ThrottledPulseSensor _legs_pulse{PULSE_PUBLISH_MIN_INTERVAL_MS};
    ThrottledPulseSensor _lumbar_pulse{PULSE_PUBLISH_MIN_INTERVAL_MS};
    MovementState _movement_state{MovementState::STOPPED};
    uint32_t _next_movement_command_ms{0};
    std::optional<uint32_t> _momentary_command_deadline_ms{};

    /* Hard limit detection */
    static constexpr uint32_t HARD_LIMIT_STARTUP_GRACE_MS = 750;
    static constexpr uint32_t HARD_LIMIT_STATIONARY_MS = 1000;
    static constexpr float HARD_LIMIT_MAX_ABS_VELOCITY = 50.0f;
    void process_hard_limit_detection_(uint32_t now_ms);
    void start_hard_limit_detection_(uint32_t now_ms);
    void reset_hard_limit_detection_();
    const ThrottledPulseSensor *active_pulse_sensor_() const;
    std::optional<uint32_t> _movement_started_ms{};
    std::optional<uint32_t> _stationary_since_ms{};
};

}  // namespace temperbridge_nova
}  // namespace esphome

#endif  // ESPHOME_TEMPERBRIDGE_NOVA_TEMPERBRIDGE_NOVA_H
