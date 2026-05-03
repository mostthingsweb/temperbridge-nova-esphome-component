```yaml
uart:
  id: mfp_uart
  tx_pin: GPIO13
  rx_pin: GPIO12
  flow_control_pin:
    number: GPIO11
    inverted: true
  baud_rate: 38400
  parity: EVEN
  stop_bits: 1
  data_bits: 8

temperbridge_nova:
  uart_id: mfp_uart
```
