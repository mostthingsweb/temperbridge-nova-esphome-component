# TemperBridge Nova ESPHome Component

External ESPHome component for TemperBridge Nova hardware (based on ESP32-S3).

## ESPHome packages

Use the base package with a model package. The CU358 package enables head and legs actuator entities and omits lumbar support.

```yaml
packages:
  temperbridge_nova_base: !include packages/temperbridge_nova/base.yaml
  temperbridge_nova_model: !include packages/temperbridge_nova/cu358.yaml
```

## Momentary actuator buttons

The component exposes Cover entities for actuator control and also exposes momentary button entities for Home Assistant dashboards. The momentary buttons are intended for `custom:button-card` with `hold_action.repeat`; firmware keeps the actuator moving while repeated button presses arrive and stops shortly after they stop.

```yaml
type: grid
columns: 2
square: false
cards:
  - type: custom:button-card
    entity: button.temperbridge_nova_head_raise
    name: Head Raise
    icon: mdi:arrow-up-bold
    tap_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_head_raise
    hold_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_head_raise
      repeat: 250

  - type: custom:button-card
    entity: button.temperbridge_nova_head_lower
    name: Head Lower
    icon: mdi:arrow-down-bold
    tap_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_head_lower
    hold_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_head_lower
      repeat: 250

  - type: custom:button-card
    entity: button.temperbridge_nova_legs_raise
    name: Legs Raise
    icon: mdi:arrow-up-bold
    tap_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_legs_raise
    hold_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_legs_raise
      repeat: 250

  - type: custom:button-card
    entity: button.temperbridge_nova_legs_lower
    name: Legs Lower
    icon: mdi:arrow-down-bold
    tap_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_legs_lower
    hold_action:
      action: call-service
      service: button.press
      target:
        entity_id: button.temperbridge_nova_legs_lower
      repeat: 250

  - type: conditional
    conditions:
      - entity: binary_sensor.temperbridge_nova_lumbar_supported
        state: "on"
    card:
      type: custom:button-card
      entity: button.temperbridge_nova_lumbar_raise
      name: Lumbar Raise
      icon: mdi:arrow-up-bold
      tap_action:
        action: call-service
        service: button.press
        target:
          entity_id: button.temperbridge_nova_lumbar_raise
      hold_action:
        action: call-service
        service: button.press
        target:
          entity_id: button.temperbridge_nova_lumbar_raise
        repeat: 250

  - type: conditional
    conditions:
      - entity: binary_sensor.temperbridge_nova_lumbar_supported
        state: "on"
    card:
      type: custom:button-card
      entity: button.temperbridge_nova_lumbar_lower
      name: Lumbar Lower
      icon: mdi:arrow-down-bold
      tap_action:
        action: call-service
        service: button.press
        target:
          entity_id: button.temperbridge_nova_lumbar_lower
      hold_action:
        action: call-service
        service: button.press
        target:
          entity_id: button.temperbridge_nova_lumbar_lower
        repeat: 250
```

Adjust the entity IDs if Home Assistant generates different names for your device.
