# Copilot Instructions for ESP32 E-Ink Timetable Display

## Project Overview
**Timetable v1.03** — A battery-powered ESP32-based device that displays daily school timetables on an e-ink display, updating from a web API and managing power consumption via deep sleep cycles.

## Architecture & Data Flow

### Core Components
- **[timetablev1.03.ino](timetablev1.03.ino)** — Main entry point; orchestrates boot sequence, data fetching, display rendering, and deep sleep
- **[classes.h](classes.h) & [classes.cpp](classes.cpp)** — Core data models: `Card` (lesson), `Period` (ring times), `Day` (date info)
- **[config.h](config.h)** — Hardware config, error codes, battery thresholds, display params, version management
- **[drawing.h](drawing.h)** — E-ink display rendering (u8g2 fonts, grid layout, dual-lesson handling)
- **[wifiLoad.h](wifiLoad.h)** — WiFi connection, OTA firmware updates, version checking
- **[periodsLoad.h](periodsLoad.h)** — Fetches ring times from `/api/api/timetable/ringsystem/` endpoint
- **[cardsLoad.h](cardsLoad.h)** — Fetches lessons from `/api/api/timetable/cards` endpoint (POST with room filter)
- **[sendBatteryData.h](sendBatteryData.h)** & **[sendEmail.h](sendEmail.h)** — Battery monitoring and alerts

### Boot Sequence & Sleep Cycle
1. **setup()** performs single execution at power-on:
   - Pin setup (GPIO5 output, ADC input for battery)
   - WiFi connection → OTA update check → Firmware install if needed
   - Fetch current date/time from server (`setClock()`)
   - Load periods and lesson cards for today
   - Calculate `timeToSleep` (seconds until next update needed)
   - Send battery level to Google Sheet (if > 10 min until next update)
   - Render display via `drawing()`
   - Enable deep sleep timer and enter `esp_deep_sleep_start()`
2. **loop()** is empty — device never reaches it during normal operation
3. Device wakes at calculated interval, repeats setup cycle

### Key Data Flows
- **API Integration**: Classroom-based filtering via `Terem` variable (set by MAC or hardcoded)
  - Periods: `GET /ringsystem/{date}`
  - Cards: `POST /cards` with JSON filter `{classroom, date}`
- **Battery Monitoring**: ADC reads voltage divider (470k/470k resistor) on GPIO36 (ESP32-E) or GPIO4 (ESP32-C6)
  - Thresholds trigger email warnings (3.45V, 3.35V, 3.25V)
- **Display Layout**: Left column (time grid, 0-9 periods), right columns (lessons, dual-column support for schedule conflicts)

## Critical Patterns & Conventions

### Error Handling
- `errorCode` (0=none, 1=WiFi fail, 2=API fail, 3=no data) drives display fallback behavior
- No WiFi → skip API, display cached/default schedule; next update deferred to next day
- API response with empty data → treats as holiday/weekend

### Hardware Variants
- **ESP32 v4.0, ESP32-E (DFR0654/DFR0478)**: ADC on GPIO36
- **ESP32-C6 (DFR1095)**: ADC on GPIO4 (external resistor divider required)
- Detect via `CONFIG_IDF_TARGET_ESP32` / `CONFIG_IDF_TARGET_ESP32C6` preprocessor directives

### Version Management
- `FW_VERSION` in [config.h](config.h) must match [version](version) file before release
- OTA update flow: check remote version, download `.bin`, auto-restart
- `bootCount` increments to detect failed refresh cycles

### Debugging
- `DEBUG` macro enables serial output (115200 baud)
- Use `DEBUG_PRINT()` / `DEBUG_PRINTLN()` — they no-op when disabled
- Hungarian comments throughout (school project context)

## Build & Update Workflow
1. **Compile** to `.bin`:
   - Arduino IDE → Export compiled binary
   - Update `FW_VERSION` in [config.h](config.h) and [version](version) file
2. **Deploy**:
   - Upload `.bin` to `http://zeus.jedlik.eu:8000/` or Koyeb endpoint
   - Update [version](version) file on same server
   - Devices auto-update on next boot if remote version > local
3. **WiFi/API Config** in [wifiLoad.h](wifiLoad.h):
   - SSID/password (currently hardcoded; change as needed)
   - Firmware and version URLs (commented alt endpoints available)

## Known Considerations
- Device sleeps most of the time; all work completes in `setup()` before sleep
- Display updates are full-refresh (e-ink latency ~1s), timed to complete before sleep
- No persistent storage; periods/lessons re-fetched every cycle (relies on API fallback for cached schedule)
- Battery voltage calibration may need tuning per PCB variant (warningAccuLevel, etc.)
- Classroom identifier (`Terem`) must align with server-side room naming

## Common Tasks
- **Add new error case**: Update `errorCode` enum in [config.h](config.h) and `errorMessages[]` array
- **Adjust display layout**: Modify grid sizing (`racs_magassag`, `time_racs_szel`) in [drawing.h](drawing.h)
- **Change update frequency**: Modify `setTimeToSleep()` logic in [utilities.h](utilities.h)
- **Support new ESP32 variant**: Add preprocessor check in [config.h](config.h) and [wifiLoad.h](wifiLoad.h)
