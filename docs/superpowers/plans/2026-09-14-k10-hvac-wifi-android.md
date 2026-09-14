# K10 HVAC Wi-Fi and Android Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a field-safe UNIHIKER K10 HVAC menu that works offline, provisions over Wi-Fi, streams authenticated device snapshots, and connects to both a standalone Android APK and the WISE² Field Tech integration surface.

**Architecture:** Pure C++ domain, navigation, serialization, pairing, and queue logic is isolated from DFRobot board APIs and host-tested. The K10 adapter uses the pinned official DFRobot Arduino core and serves `/api/v1` REST plus a WebSocket stream. A shared TypeScript package owns Android discovery, pairing, validation, reconnection, and persistence; a Capacitor app and a Field Tech adapter both consume it.

**Tech Stack:** C++17, DFRobot UNIHIKER K10 Arduino SDK, Arduino CLI, Node.js 24, npm workspaces, TypeScript, Vitest, React, Vite, Capacitor Android, Java 17.

**Spec:** `docs/superpowers/specs/2026-09-13-k10-hvac-menu-design.md`

## Global Constraints

- Target the DFRobot UNIHIKER K10 through the official board package URL `https://downloadcd.dfrobot.com.cn/UNIHIKER/package_unihiker_index.json`; never assume ESP32 compatibility.
- Production builds must never label fixtures as measured data.
- All unverified physical inputs render `DISCONNECTED` or `UNSUPPORTED`.
- Never connect thermostat, mains, pressure, current, inductive, or long-wire inputs directly to K10 GPIO.
- Use 2.4 GHz station Wi-Fi first and password-protected `WISE2-K10-<last4>` setup AP fallback.
- Pair with a short-lived six-digit code; store tokens through the Android secure-store interface; commit no secrets.
- Cloud upload is disabled unless an API base URL and device credential are provisioned.
- Existing Field Tech source is absent; deliver a tested integration package and example screen without claiming its external APK was rebuilt.
- Do not claim physical readiness until USB detection, backup, flash, three warm reboots, one power cycle, and live K10-to-Android sync pass.

---

### Task 1: Repository foundation and executable contracts

**Files:**
- Create: `package.json`
- Create: `.gitignore`
- Create: `contracts/wise2-device-api/snapshot.schema.json`
- Create: `contracts/wise2-device-api/pair.schema.json`
- Create: `contracts/wise2-device-api/event.schema.json`
- Create: `packages/contracts/package.json`
- Create: `packages/contracts/src/index.ts`
- Test: `packages/contracts/test/contracts.test.ts`

**Interfaces:**
- Produces: `Measurement`, `DeviceSnapshot`, `PairRequest`, `PairResponse`, and `DeviceEvent` TypeScript types; JSON Schemas with `schemaVersion: 1`.
- Consumes: no application code.

- [ ] **Step 1: Write the failing contract tests**

```ts
import { describe, expect, it } from "vitest";
import { parseSnapshot, parsePairResponse } from "../src/index";

describe("device contracts", () => {
  it("rejects a measured value without sensor identity and timestamp", () => {
    expect(() => parseSnapshot({ schemaVersion: 1, measurements: [{ value: 72, origin: "measured" }] })).toThrow();
  });
  it("accepts an explicitly disconnected channel without a numeric value", () => {
    expect(parseSnapshot({ schemaVersion: 1, deviceId: "k10-ab12", capturedAt: "2026-09-14T01:00:00Z", measurements: [{ id: "supply-air", unit: "degF", origin: "measured", validity: "disconnected", capturedAt: "2026-09-14T01:00:00Z" }] }).measurements[0].validity).toBe("disconnected");
  });
  it("rejects an expired pairing response", () => {
    expect(() => parsePairResponse({ token: "x", expiresAt: "2020-01-01T00:00:00Z" }, new Date("2026-09-14T01:00:00Z"))).toThrow("expired");
  });
});
```

- [ ] **Step 2: Run the contract test and verify RED**

Run: `npm install && npm test -- packages/contracts/test/contracts.test.ts`
Expected: FAIL because `parseSnapshot` and `parsePairResponse` do not exist.

- [ ] **Step 3: Implement strict schema-backed parsing**

Create the three version-1 schemas with `additionalProperties: false`, the six allowed validity states, the three allowed origins, ISO timestamps, and stable event IDs. Export parsers that return typed values and throw descriptive validation errors. Pair parsing must compare `expiresAt` against the injected `Date`.

- [ ] **Step 4: Run tests and verify GREEN**

Run: `npm test -- packages/contracts/test/contracts.test.ts`
Expected: 3 tests pass with zero warnings.

- [ ] **Step 5: Commit**

```bash
git add package.json .gitignore contracts packages/contracts
git commit -m "feat: define K10 device contracts"
```

### Task 2: Host-tested K10 domain and navigation core

**Files:**
- Create: `devices/k10-hvac/include/wise2/domain.hpp`
- Create: `devices/k10-hvac/include/wise2/navigation.hpp`
- Create: `devices/k10-hvac/src/domain/domain.cpp`
- Create: `devices/k10-hvac/src/ui/navigation.cpp`
- Test: `devices/k10-hvac/test/domain_test.cpp`
- Test: `devices/k10-hvac/test/navigation_test.cpp`
- Create: `devices/k10-hvac/test/run-host-tests.sh`

**Interfaces:**
- Produces: `wise2::Measurement`, `wise2::Snapshot`, `wise2::Screen`, `wise2::Input`, `wise2::nextScreen(Screen, Input)`, and `wise2::displayValue(const Measurement&, uint64_t nowMs)`.
- Consumes: version-1 names from Task 1 contracts.

- [ ] **Step 1: Write failing host tests**

```cpp
assert(displayValue({"supply-air", 0.0f, "degF", 1000, Origin::Measured, Validity::Disconnected}, 1000) == "DISCONNECTED");
assert(displayValue({"return-air", 75.2f, "degF", 1000, Origin::Measured, Validity::Valid}, 1000) == "75.2 °F");
assert(nextScreen(Screen::Home, Input::TapGauges) == Screen::LiveGauges);
assert(nextScreen(Screen::Electrical, Input::ButtonA) == Screen::LiveGauges);
assert(nextScreen(Screen::Alerts, Input::ButtonHome) == Screen::Home);
```

- [ ] **Step 2: Run host tests and verify RED**

Run: `bash devices/k10-hvac/test/run-host-tests.sh`
Expected: compilation fails because the domain/navigation headers do not exist.

- [ ] **Step 3: Implement the minimal domain and deterministic navigation table**

Define all ten screens and physical/touch inputs. Format valid values by unit; render every non-valid state as its uppercase state and never reuse a previous numeric value.

- [ ] **Step 4: Run host tests and verify GREEN**

Run: `bash devices/k10-hvac/test/run-host-tests.sh`
Expected: both test binaries exit 0.

- [ ] **Step 5: Commit**

```bash
git add devices/k10-hvac/include devices/k10-hvac/src/domain devices/k10-hvac/src/ui devices/k10-hvac/test
git commit -m "feat: add K10 domain and menu navigation"
```

### Task 3: Pairing, authentication, and offline queue core

**Files:**
- Create: `devices/k10-hvac/include/wise2/pairing.hpp`
- Create: `devices/k10-hvac/include/wise2/event_queue.hpp`
- Create: `devices/k10-hvac/src/connectivity/pairing.cpp`
- Create: `devices/k10-hvac/src/storage/event_queue.cpp`
- Test: `devices/k10-hvac/test/pairing_test.cpp`
- Test: `devices/k10-hvac/test/event_queue_test.cpp`

**Interfaces:**
- Produces: `PairingSession::issue(nowMs, entropy)`, `PairingSession::exchange(code, nowMs)`, `PairingSession::authorize(token, nowMs)`, `EventQueue::enqueue(DeviceEvent)`, `EventQueue::ack(eventId)`, and `EventQueue::pending()`.
- Consumes: stable event IDs and `schemaVersion: 1` from Task 1.

- [ ] **Step 1: Write failing pairing and queue tests**

```cpp
auto session = PairingSession::issue(1000, 123456);
assert(session.code() == "123456");
assert(session.exchange("123456", 61001).status == PairStatus::Expired);
assert(session.exchange("000000", 2000).status == PairStatus::Rejected);
EventQueue q;
assert(q.enqueue({"evt-1", 1, "work-order.updated"}));
assert(!q.enqueue({"evt-1", 1, "work-order.updated"}));
q.ack("evt-1");
assert(q.pending().empty());
```

- [ ] **Step 2: Run host tests and verify RED**

Run: `bash devices/k10-hvac/test/run-host-tests.sh`
Expected: compilation fails on missing pairing and queue types.

- [ ] **Step 3: Implement sixty-second pairing and idempotent queue behavior**

Hash tokens before persistence, use constant-time token comparison, cap failed code attempts at five per session, and retain unacknowledged events in insertion order. Keep storage behind an interface so the host uses memory and K10 uses atomic files/preferences.

- [ ] **Step 4: Run host tests and verify GREEN**

Run: `bash devices/k10-hvac/test/run-host-tests.sh`
Expected: all domain, navigation, pairing, and queue tests exit 0.

- [ ] **Step 5: Commit**

```bash
git add devices/k10-hvac
git commit -m "feat: add secure K10 pairing and event queue"
```

### Task 4: K10 board adapter, Wi-Fi service, and ten-screen UI

**Files:**
- Create: `devices/k10-hvac/arduino-cli.yaml`
- Create: `devices/k10-hvac/wise2_k10_hvac.ino`
- Create: `devices/k10-hvac/include/wise2/board.hpp`
- Create: `devices/k10-hvac/src/drivers/k10_board.cpp`
- Create: `devices/k10-hvac/src/connectivity/http_api.cpp`
- Create: `devices/k10-hvac/src/connectivity/wifi_manager.cpp`
- Create: `devices/k10-hvac/src/ui/screens.cpp`
- Create: `devices/k10-hvac/src/storage/k10_storage.cpp`
- Create: `devices/k10-hvac/docs/FLASHING.md`
- Test: `devices/k10-hvac/test/http_api_test.cpp`
- Test: `devices/k10-hvac/test/wifi_manager_test.cpp`

**Interfaces:**
- Produces: `GET /api/v1/health`, `GET /api/v1/snapshot`, `POST /api/v1/pair`, `POST /api/v1/events`, `WS /api/v1/stream`, and `_wise2-k10._tcp` advertisement.
- Consumes: Tasks 1-3 contracts and domain APIs.

- [ ] **Step 1: Write failing adapter-state tests**

```cpp
FakeRadio radio;
WifiManager wifi(radio, "k10-ab12");
wifi.start({});
assert(radio.apSsid() == "WISE2-K10-ab12");
assert(radio.apPassword().size() >= 12);
auto unauthorized = route({"GET", "/api/v1/snapshot", ""});
assert(unauthorized.status == 401);
auto health = route({"GET", "/api/v1/health", ""});
assert(health.status == 200);
```

- [ ] **Step 2: Run host tests and verify RED**

Run: `bash devices/k10-hvac/test/run-host-tests.sh`
Expected: compilation fails because Wi-Fi manager and router are missing.

- [ ] **Step 3: Implement board-independent routing and Wi-Fi state machine**

Keep `/health` unauthenticated and redact secrets. Require a bearer device token everywhere else except `/pair`. Attempt saved station credentials with a bounded timeout, then start the protected AP. Rate-limit pairing and event routes.

- [ ] **Step 4: Implement the K10 adapter and all ten screens**

Use the official K10 display/touch/button APIs. Build Home, Live Gauges, Psychrometrics, Refrigerant, Electrical, Diagnostics, Work Order, Sensors, Alerts, and Device screens from immutable snapshots. Gray disconnected cards are the boot default. Device screen shows Wi-Fi state, IP, pairing code expiry, queue depth, firmware version, and last sync.

- [ ] **Step 5: Verify host tests, install the pinned board core, and compile**

Run:
```bash
bash devices/k10-hvac/test/run-host-tests.sh
arduino-cli config init --overwrite
arduino-cli config add board_manager.additional_urls https://downloadcd.dfrobot.com.cn/UNIHIKER/package_unihiker_index.json
arduino-cli core update-index
arduino-cli board listall | rg -i 'unihiker|k10'
```
Resolve the exact FQBN from the installed board index and fail when none is present:

```bash
K10_FQBN="$(arduino-cli board listall | awk 'tolower($0) ~ /unihiker.*k10/ {print $NF; exit}')"
test -n "$K10_FQBN"
arduino-cli compile --fqbn "$K10_FQBN" devices/k10-hvac
```

Save the full output. Do not guess the FQBN or claim a build if the official package cannot be installed.

- [ ] **Step 6: Commit**

```bash
git add devices/k10-hvac
git commit -m "feat: build K10 UI and local Wi-Fi API"
```

### Task 5: Shared Android sync core

**Files:**
- Create: `packages/k10-sync-core/package.json`
- Create: `packages/k10-sync-core/src/client.ts`
- Create: `packages/k10-sync-core/src/discovery.ts`
- Create: `packages/k10-sync-core/src/queue.ts`
- Create: `packages/k10-sync-core/src/secure-store.ts`
- Create: `packages/k10-sync-core/src/index.ts`
- Test: `packages/k10-sync-core/test/client.test.ts`
- Test: `packages/k10-sync-core/test/queue.test.ts`

**Interfaces:**
- Produces: `K10Client.discover()`, `K10Client.pair(baseUrl, code)`, `K10Client.connect()`, `K10Client.subscribe(listener)`, `K10Client.enqueue(event)`, and `K10Client.flush()`.
- Consumes: Task 1 parsers and injected `SecureStore`, `Discovery`, `HttpTransport`, `SocketTransport`, and `QueueStore` interfaces.

- [ ] **Step 1: Write failing client tests**

```ts
it("stores a token only after a valid unexpired pair response", async () => {
  const store = memorySecureStore();
  const client = testClient({ store, pairResponse: validPairResponse });
  await client.pair("http://192.168.4.1", "123456");
  expect(await store.get("k10-ab12")).toBe(validPairResponse.token);
});
it("retains an event after a network failure and removes it after acknowledgment", async () => {
  const client = testClient({ eventResponses: [new Error("offline"), { acknowledged: ["evt-1"] }] });
  await client.enqueue(event("evt-1"));
  await expect(client.flush()).rejects.toThrow("offline");
  expect(await client.pending()).toHaveLength(1);
  await client.flush();
  expect(await client.pending()).toHaveLength(0);
});
```

- [ ] **Step 2: Run tests and verify RED**

Run: `npm test -- packages/k10-sync-core`
Expected: FAIL because `K10Client` is missing.

- [ ] **Step 3: Implement the injectable sync core**

Validate every network response, never log tokens or Wi-Fi passwords, reconnect WebSocket with bounded jittered backoff, reject schema versions other than 1, and deduplicate queued events by ID.

- [ ] **Step 4: Run tests and verify GREEN**

Run: `npm test -- packages/k10-sync-core`
Expected: all sync tests pass with zero unhandled rejections.

- [ ] **Step 5: Commit**

```bash
git add packages/k10-sync-core
git commit -m "feat: add shared K10 Android sync core"
```

### Task 6: Standalone K10 Companion Android app

**Files:**
- Create: `apps/k10-companion/package.json`
- Create: `apps/k10-companion/capacitor.config.ts`
- Create: `apps/k10-companion/src/App.tsx`
- Create: `apps/k10-companion/src/theme.css`
- Create: `apps/k10-companion/src/screens/PairScreen.tsx`
- Create: `apps/k10-companion/src/screens/DashboardScreen.tsx`
- Create: `apps/k10-companion/src/screens/DeviceScreen.tsx`
- Test: `apps/k10-companion/src/App.test.tsx`

**Interfaces:**
- Produces: Android application ID `net.wise2.k10companion` and a debug APK.
- Consumes: `K10Client` from Task 5.

- [ ] **Step 1: Write failing UI tests**

```tsx
it("shows disconnected before a real snapshot arrives", () => {
  render(<App client={disconnectedClient()} />);
  expect(screen.getByText("K10 DISCONNECTED")).toBeVisible();
  expect(screen.queryByText(/72\.0/)).not.toBeInTheDocument();
});
it("pairs with the six-digit code and renders validated values", async () => {
  render(<App client={connectedClient(validSnapshot)} />);
  await userEvent.type(screen.getByLabelText("Pairing code"), "123456");
  await userEvent.click(screen.getByRole("button", { name: "PAIR K10" }));
  expect(await screen.findByText("75.2 °F")).toBeVisible();
});
```

- [ ] **Step 2: Run UI tests and verify RED**

Run: `npm test -- apps/k10-companion`
Expected: FAIL because the app screens do not exist.

- [ ] **Step 3: Implement the field-readable UI**

Use the approved deep navy/black, chrome, cyan, orange, green, amber, red, and gray system. Provide Pair, Home, Gauges, Diagnostics, Job, and Device routes; expose all ten K10 screen summaries from the More menu. Keep touch targets at least 48 CSS pixels.

- [ ] **Step 4: Run UI tests and production web build**

Run: `npm test -- apps/k10-companion && npm run build -w apps/k10-companion`
Expected: tests pass and Vite exits 0.

- [ ] **Step 5: Add Android and build the debug APK**

Run:
```bash
npx cap add android --workspace apps/k10-companion
npm run cap:sync -w apps/k10-companion
cd apps/k10-companion/android && ./gradlew assembleDebug
```
Expected artifact: `apps/k10-companion/android/app/build/outputs/apk/debug/app-debug.apk` with package `net.wise2.k10companion`.

- [ ] **Step 6: Commit**

```bash
git add apps/k10-companion packages/k10-sync-core
git commit -m "feat: add K10 Companion Android app"
```

### Task 7: WISE² Field Tech integration module

**Files:**
- Create: `integrations/field-tech-k10/package.json`
- Create: `integrations/field-tech-k10/src/K10Provider.tsx`
- Create: `integrations/field-tech-k10/src/K10Panel.tsx`
- Create: `integrations/field-tech-k10/src/index.ts`
- Create: `integrations/field-tech-k10/README.md`
- Test: `integrations/field-tech-k10/test/K10Panel.test.tsx`

**Interfaces:**
- Produces: `K10Provider`, `useK10()`, and `K10Panel` for embedding in the existing Field Tech app.
- Consumes: `K10Client` from Task 5 and version-1 contracts from Task 1.

- [ ] **Step 1: Write the failing integration test**

```tsx
it("maps a K10 snapshot into a Field Tech work-order attachment", async () => {
  render(<K10Provider client={connectedClient(validSnapshot)}><K10Panel workOrderId="wo-42" /></K10Provider>);
  await userEvent.click(await screen.findByRole("button", { name: "ATTACH READINGS" }));
  expect(onAttach).toHaveBeenCalledWith(expect.objectContaining({ workOrderId: "wo-42", deviceId: "k10-ab12", schemaVersion: 1 }));
});
```

- [ ] **Step 2: Run integration test and verify RED**

Run: `npm test -- integrations/field-tech-k10`
Expected: FAIL because `K10Provider` and `K10Panel` are missing.

- [ ] **Step 3: Implement the provider, panel, and integration guide**

Expose connection state, latest validated snapshot, queue depth, pair action, and attach-readings action. Document the exact install/import snippet and Android local-network permissions. Do not duplicate transport or token code.

- [ ] **Step 4: Run tests and package build**

Run: `npm test -- integrations/field-tech-k10 && npm run build -w integrations/field-tech-k10`
Expected: tests pass and TypeScript emits declarations without errors.

- [ ] **Step 5: Commit**

```bash
git add integrations/field-tech-k10
git commit -m "feat: add Field Tech K10 integration module"
```

### Task 8: End-to-end verification, artifacts, and field handoff

**Files:**
- Create: `scripts/verify-k10.sh`
- Create: `devices/k10-hvac/docs/FIELD-CHECKLIST.md`
- Create: `devices/k10-hvac/docs/KNOWN-LIMITATIONS.md`
- Create: `artifacts/checksums.sha256`
- Modify: `README.md`

**Interfaces:**
- Produces: one repeatable verification command, checksums, rollback instructions, and an evidence table separating host, build, USB, flash, and live-sync results.
- Consumes: all earlier tasks.

- [ ] **Step 1: Write the verification script test**

Create a shell test that runs `scripts/verify-k10.sh --host-only`, asserts a nonzero exit if any contract, C++, UI, integration, or web build fails, and asserts the script prints `HARDWARE: NOT TESTED` when no serial device is present.

- [ ] **Step 2: Run the script test and verify RED**

Run: `bash test/verify-k10-script.test.sh`
Expected: FAIL because `scripts/verify-k10.sh` does not exist.

- [ ] **Step 3: Implement the verifier and field documents**

The verifier runs every host test and build, discovers but never guesses the K10 FQBN/port, checks `adb devices -l`, hashes the APK and firmware outputs, and writes no credentials. The field checklist includes power-on, AP fallback, station join, pairing expiry, invalid-code rejection, snapshot stream, event retry, three warm reboots, one power cycle, and rollback. Mark every unrun physical item `BLOCKED — DEVICE NOT PRESENT`.

- [ ] **Step 4: Run full available verification**

Run: `bash scripts/verify-k10.sh --host-only`
Expected: all available host tests/builds pass; hardware status is explicitly not tested when devices are absent.

- [ ] **Step 5: If devices are reachable, back up, flash, install, and test**

Run `arduino-cli board list` and `adb devices -l`; record exact identifiers. Back up the K10 with the official supported method before flashing. Compile/upload using only the discovered FQBN and port. Install with `adb install -r apps/k10-companion/android/app/build/outputs/apk/debug/app-debug.apk`. Execute every physical field-checklist row and record actual pass/fail evidence.

- [ ] **Step 6: Commit and push**

```bash
git add README.md scripts test devices/k10-hvac/docs artifacts
git commit -m "docs: add K10 field verification and artifacts"
git push origin HEAD
```

- [ ] **Step 7: Verify the remote commit**

Fetch the GitHub branch head, confirm it equals local `git rev-parse HEAD`, confirm the APK/firmware checksums match `artifacts/checksums.sha256`, and report physical-device items separately from host/build items.
