# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
