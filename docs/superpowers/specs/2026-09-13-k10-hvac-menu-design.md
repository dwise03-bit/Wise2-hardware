# WISE² HVAC Pocket Node — K10 Menu Design

**Status:** Approved 2026-09-13  
**Product:** WISE² HVAC Pocket Node — K10 Edition  
**Target:** DFRobot UNIHIKER K10  

## Objective

Build a production embedded application that boots directly into the approved ten-screen WISE² HVAC menu, remains fully usable offline, reports only real validated sensor data on hardware, and synchronizes through versioned WISE² Core device APIs.

## Product Boundary

The first release includes the menu shell, navigation, domain models, formatting, status semantics, persistent local settings, hardware-adapter interfaces, simulator fixtures, and API contract stubs. Real field sensors are enabled only after their protected interface hardware, pin assignments, calibration procedure, and driver tests are verified.

No Discord alert is posted during development. The device may prepare an alert payload for review, but external transmission requires explicit approval and configured credentials.

## Architecture

The firmware uses C++17, FreeRTOS, LVGL, and a pinned PlatformIO toolchain. UI code consumes immutable domain snapshots and never reads GPIO directly. Drivers publish typed readings through the sensor service; storage, networking, and synchronization operate behind independent interfaces so the menu remains responsive during failures.

The Mac simulator compiles the same navigation, view-model, formatting, and status logic as the device build. Simulator fixtures are explicitly marked `SIMULATED`; production builds do not contain a path that can present fixture values as measured data.

```text
Hardware Drivers -> Sensor Service -> Domain Snapshot -> View Models -> LVGL Screens
                          |                  |
                          v                  v
                    Local Storage       Alert Evaluator
                          |                  |
                          +---- Sync Queue --+----> WISE² Core API
```

## Repository Layout

```text
wise2-hardware/
├── devices/k10-hvac/
│   ├── include/wise2/
│   ├── src/domain/
│   ├── src/ui/
│   ├── src/drivers/
│   ├── src/connectivity/
│   ├── src/storage/
│   ├── simulator/
│   ├── test/
│   ├── docs/
│   └── platformio.ini
├── contracts/wise2-device-api/
└── assets/k10-hvac/
```

## Menu System

The global bottom navigation contains `HOME`, `GAUGES`, `DIAG`, `JOB`, and `MORE`. The ten screens are:

1. **Home:** system mode, supply, return, delta-T, humidity, connectivity, sensor count, power state, and active alerts.
2. **Live Gauges:** air, refrigerant pressure, electrical, and condensate groups.
3. **Psychrometrics:** dry bulb, wet bulb, relative humidity, dew point, enthalpy, and delta values.
4. **Refrigerant:** explicit refrigerant selection, low/high pressure, saturation temperatures, line temperatures, superheat, and subcooling.
5. **Electrical:** isolated G, Y1, Y2, W1, W2, and O/B call states plus validated compressor and blower current.
6. **Diagnostics:** deterministic findings separated into verified, warning, and fault groups with evidence and confirmation steps.
7. **Work Order:** customer, site, asset, notes, photo, voice memo, save, and synchronization state.
8. **Sensors:** channel identity, connection, calibration, freshness, and fault state.
9. **Alerts:** active and resolved alerts; acknowledgment never erases the audit record.
10. **Device:** Wi-Fi, BLE, API, storage, firmware, calibration, diagnostics, and safe update.

Touch gestures are tap to open, hold for details, and horizontal swipe between sibling views. Physical controls are A for back, Home for dashboard, and B for the screen's labeled contextual action. Every action remains reachable without gestures.

## Visual System

- Deep navy and black base with metallic silver WISE² identity.
- Electric blue means normal information.
- HVAC orange means an active equipment call or active function.
- Green means verified healthy.
- Amber means attention or warning.
- Red means fault or dangerous condition only.
- Gray means disconnected, unavailable, or offline.
- Rounded panels, restrained glow, large field-readable values, and gloved-use touch targets.
- No emojis, generic robots, fake holograms, Arduino/debug clutter, or unverified claims.

The approved master sheet is stored as `assets/k10-hvac/k10-ui-master-sheet.png`.

## Domain Model

Each measurement contains value, unit, timestamp, sensor identifier, origin, calibration state, and validity state. Validity is one of `valid`, `stale`, `disconnected`, `uncalibrated`, `unsupported`, or `faulted`. Origin is `measured`, `calculated`, or `simulated`.

Calculated values are valid only when every required input is valid and fresh. Production screens render invalid states instead of retaining the last valid number without a stale marker.

## Safety Boundary

No thermostat voltage, mains voltage, pressure transducer, current transformer, inductive load, or long field wire connects directly to K10 GPIO. Field inputs require documented isolation, fusing or resettable protection, transient protection, signal conditioning, connector pinout, grounding, and voltage-range verification.

The initial firmware exposes adapter interfaces for supply/return psychrometers, refrigerant pressure, clamp current, isolated 24 VAC calls, and condensate detection. A driver is enabled in production only after its hardware path is documented and tested.

## Offline and Synchronization Behavior

The menu boots without Wi-Fi or API access. Settings and queued events use versioned, atomic local records. Uploads use stable event identifiers so retries cannot create duplicate CRM records. Reconnection uses bounded exponential backoff and never blocks display input.

WISE² Core remains the system of record for customers, sites, assets, and work orders. The hardware repository owns device firmware and shared device payload schemas, not duplicate customer data.

## Error Handling

- Missing optional hardware produces `UNSUPPORTED` or `DISCONNECTED`, never a fabricated reading.
- Stale data remains visible only with a clear stale state and timestamp.
- Storage write failure preserves the active UI and raises a local fault.
- Authentication failure stops uploads but retains the queue.
- Repeated boot failures enter a safe diagnostic screen.
- Watchdog recovery records the reset cause.
- Configuration migrations are versioned and fail closed to safe defaults.

## Testing

Host tests cover navigation transitions, physical-button behavior, value formatting, color/status mapping, invalid and stale readings, calculated-value prerequisites, persistence, queue idempotency, and API serialization. Device verification covers display, touch, buttons, storage, Wi-Fi, BLE when supported, reboot recovery, and missing-sensor behavior.

No hardware result is claimed without a fresh real-device test. Flashing requires exact board and port detection, a verified flash backup with SHA-256 hashes, a clean build, and passing host tests.

## Delivery Sequence

1. Create the repository foundation and pinned build environments.
2. Implement and test domain types and status semantics.
3. Implement and test navigation and input mapping.
4. Build the ten LVGL screens against view models.
5. Add atomic local settings and offline event queue.
6. Add hardware adapter interfaces with disconnected defaults.
7. Add versioned WISE² API payloads and disabled-by-default sync.
8. Detect, back up, build for, and flash the real K10.
9. Run the device verification checklist and document actual results.

## Definition of Done

- The real K10 boots directly into the WISE² HVAC Home screen.
- All ten screens are reachable by touch and physical buttons.
- The visual hierarchy matches the approved master sheet within real display constraints.
- Production firmware cannot show simulator fixtures as measured data.
- Missing, stale, uncalibrated, unsupported, and faulted sensors render correctly.
- Offline settings and queued events survive reboot.
- Host tests and the pinned device build pass.
- Three warm reboots and one power cycle succeed on the real K10.
- API synchronization is verified or explicitly reported as blocked.
- Backups, hashes, rollback commands, wiring safety, and known limitations are documented.
- No credentials are committed or displayed.

