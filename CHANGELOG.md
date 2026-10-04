# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.5.0-beta.2] - 2026-10-04

### Fixed
- **Heater could stay on until the steam thermostat tripped.** The temperature
  spike filter in `Sensor.cpp` latched: once the real temperature moved more
  than 1 C away from the last accepted sample (e.g. during a stall of the main
  loop while heating up), every following sample was rejected forever. The PID
  kept using a frozen temperature and drove the heater at 100 %. Reproduced by
  blackholing packets to the MQTT broker while heating, and verified fixed
  with the same test. The filter now holds back spikes only until they are
  confirmed by consecutive samples, so the reading always recovers.
- Blocking MQTT connect / DNS lookup (unreachable broker) stalled the control
  loop for seconds. MQTT now runs in its own FreeRTOS task; the main loop only
  hands over the status string and defers config saving.
- Possible buffer overflow when MQTT settings were longer than their buffers
  (`strcpy` replaced with bounded `strlcpy`).

### Added
- **Failsafe**: heater is forced off when no valid temperature sample arrives
  for 2 s (sensor fault, invalid/NaN reading) or when the temperature exceeds
  `MAX_SAFE_TEMP` (175 C, above normal steam-mode temperature). The PID is
  reset on recovery to avoid integral windup. State exposed as `sensorFault`
  in `/api/status`.
- **Heater hardware-timer watchdog**: the SSR pin is forced low by an
  `esp_timer` if the main loop does not refresh the heater within 1.5 s.
- **Task watchdog** (10 s) for the main loop, also fed during OTA upload.
- SSR pin is driven low first thing at boot, before WiFiManager or SPIFFS.
- Plausibility check of sensor readings (-20 .. 250 C).

### Changed
- MQTT initialisation (`setupMQTT()`) now only requests (re)configuration; the
  MQTT task is created once.
- Bumped firmware version to `1.5.0-beta.2`.

---

## [1.4.0] - 2026-09-12

### Added
- **Web UI MQTT Configuration**: Added configuration fields for MQTT server (`mqtt_server`), port (`mqtt_port`), username (`mqtt_user`), and password (`mqtt_pass`) directly in `/config.html`.
- **Auto-reconnect on Config Update**: Updating MQTT settings via `/set_config` now reinitializes MQTT client connection and reconnects immediately to the new broker.
- **Auto-save on Web Config**: Submitting settings on `/set_config` automatically persists them to SPIFFS `/config.json`.

### Changed
- Bumped firmware version to `1.4.0`.
- Regenerated `src/WebStatic.h` with new UI elements.

---

## [1.3.2] - 2026-03-09

### Changed
- Updated UI buttons spacing and responsive layout improvements.
- Updated documentation and screenshots with ECO timer feature.

---

## [1.3.1] - 2026-03-08

### Added
- ECO feature (automatic heater turn-off timer).
- Injected git commit hash and build timestamp into `/api/config` and web UI footer.

---

## [1.3.0] - 2026-03-05

### Added
- Configurable MQTT topic and MQTT enable/disable toggle in Web UI.
- Real-time chart telemetry with 180 data points.

---

## [1.2.0] - 2026-02-20

### Added
- Over-The-Air (OTA) firmware update via `/update`.
- Web-based PID tuning controls and live temperature graphs.

---

## [1.1.0] - 2026-02-10

### Added
- Initial support for ESP32-C6 DevKitC-1 with MAX31865 RTD PT100/PT1000 sensor.
- WiFiManager AP setup mode (`ESPressIoT-Setup`).
- Basic web dashboard and REST API.
