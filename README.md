# GMLAN-to-HolleyEFI-converter
Reads certain parameters from GM E38 ECU and sends them as CAN inputs to a Holley EFI.
This is firmware for an [AutoSportLabs ESP32-CAN-X2 board](https://www.autosportlabs.com/product/esp32-can-x2-dual-can-bus-automotive-grade-development-board/) and will send several factory messages to a Holley EFI system over CAN.

## Connections
- CAN 1 connects to the vehicle CAN bus, this can be tapped directly in, or use the OBD-II port.
- CAN 2 connects to a Holley EFI CAN bus and can be pinned into a MUZI-P25 female connector.
- USB-C can connect to [USB-C to 12V converter](https://www.amazon.com/dp/B09DGDQ48H) for the easiest power connection.

## CAN Setup
The follow CAN PIDS will be sent from the E38 ECU to the Holley EFI system.
- Fuel Level
  - CAN Device CAN I/O Module
  - CAN ID 1
  - CAN INPUT 1
  - CAN Type: CAN Thermistor - Custom Therm
    - Scale (0-100 %)
    - Ohms (0-100)
    - Broadcast Rate 0.5 Hz
- AC Compressor Status
  - CAN Device CAN I/O Module
  - CAN ID 1
  - CAN INPUT 2
  - CAN Type: CAN Ground
    - Broadcast Rate 0.5 Hz
- AC High Pressure
  - CAN Device CAN I/O Module
  - CAN ID 1
  - CAN INPUT 3
  - CAN Type CAN Thermistor - Custom Therm
    - Scale (0-545 psi)
    - Ohms (0-545)
    - Broadcast Rate 0.5 Hz
- Reduced Power
  - CAN Device CAN I/O Module
  - CAN ID 1
  - CAN INPUT 4
  - CAN Type CAN Ground 
    - Broadcast Rate 0.5 Hz
- Outside Air Temp (F)
  - CAN Device CAN I/O Module
  - CAN ID 1
  - CAN INPUT 5
  - CAN Type CAN Thermistor - Custom Therm
    - Scale (0 - 150)
    - Ohms (0 - 150)
    - Broadcast Rate 0.5 Hz
