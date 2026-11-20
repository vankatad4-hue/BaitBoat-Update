# Google Online Map Renderer — Phase 1 Integration Contract (VER029.7)

**Status:** Phase 1 settings UI partly implemented; no Google tile requests or online rendering code enabled.
**Baseline:** Remote ver028 / Boat ver014, branch `hardware-ready-dual-radio-r1`.
**Policy:** Google Maps imagery may only be displayed online under applicable Maps Tile API terms. No persistent tile cache, SD storage, bulk requests, tile stitching/export, or image analysis.

## Current code audit
- Wi-Fi settings already allow SSID/password entry, NVS storage and a user-started Wi-Fi connection.
- There is no online map provider, Google Maps Tile API session handling, internet reachability test, credential provisioning, image decoder integration, online/offline source selector, or online renderer.
- The current renderer is SD-only, using 9 RGB565 tile slots and 256px tiles. Its renderer attribution is sourced from offline pack manifest.
- Wi-Fi association alone must not be considered internet/API availability.

## Required design decisions
1. Map-source mode: AUTO / ONLINE / OFFLINE. Persist preference; default AUTO. OFFLINE must never issue network requests. ONLINE displays explicit unavailable state when internet/API is unavailable. AUTO uses online only when provider requests are healthy and falls back to verified SD coverage.
2. Layer choice: ONLINE ROADMAP / SATELLITE. Default SATELLITE for fishing; ROADMAP available. Offline style choice remains separately controlled by installed pack.
3. Network requests run in a dedicated worker/task, not LVGL event callbacks or control/radio loop. Requests must have finite connect/read timeouts and bounded retry/backoff.
4. Use TLS with certificate validation. No insecure client mode. API key/session must never be logged, committed, or placed in public source. Credentials require user provisioning; key restrictions and quota/billing must be configured externally.
5. Google session lifecycle: create session for requested layer/options; tile requests use only the current session; expiry/auth/quota failures invalidate session and report service state. Request only tiles required by visible viewport.
6. Decoder: choose a maintained ESP32-S3-compatible JPEG decoder after checking target libraries and memory. Decode directly into transient PSRAM-backed RGB565 display buffers. Avoid persistent image/tile cache. Release buffers when viewport changes or source changes.
7. Display attribution is mandatory and must remain readable in map split and fullscreen. Show ONLINE/SD source, layer and service/error indicator without obscuring waypoint/boat/route overlays.
8. Never block autopilot, radio, joystick sampling, sonar or LVGL. Map failures are presentation-only and do not affect navigation/safety.
9. AUTO fallback decision requires confirmed request health, not merely Wi-Fi connected. When offline coverage is absent, retain overlays on neutral background and show no-map warning.

## Proposed implementation phases
- **P1 — provider contracts and source settings:** source/layer enums, NVS preference, UI status, interfaces, no network request.
- **P2 — connectivity worker:** internet probe and network-state events; test AP connected/no internet and loss/recovery.
- **P3 — API credentials and session:** key provisioning UI or documented provisioning path, session creation, TLS, token expiry/errors.
- **P4 — transient tile request/decode/display:** visible viewport only; no persistent cache; memory caps and attribution.
- **P5 — source selection/fallback integration:** AUTO/ONLINE/OFFLINE and verified offline coverage.
- **P6 — validation:** host tests for state transitions; target build; API quota/error/network loss; memory pressure; UI/control responsiveness; attribution and applicable policy review.

## Release gate
No Google online mode shall be advertised as functional until API key provisioning, external billing/quota configuration, TLS certificate validation, decoder, online viewport rendering and target tests are verified. No API key or test credential may enter repository files or release ZIPs.


## P1 implementation note (2026-11-20)
- The Settings → MAP UI now cycles and displays persisted AUTO / ONLINE / OFFLINE preference.
- It explains AUTO fallback intent, offline-only behavior and that Wi-Fi association does not prove internet access.
- The UI explicitly marks Google provider unavailable and unconfigured.
- P1 is only partially complete: preference presentation is wired; provider/service interface and internet-health reporting are still absent.

\n## P2 groundwork note (2026-11-20)\n- MAP settings now polls Wi-Fi association every 2 seconds using a UI timer and displays Internet reachability as UNKNOWN.\n- Polling is nonblocking and the timer is deleted on category changes to avoid dangling LVGL label callbacks.\n- This is not a true internet probe; P2 still requires a bounded, worker-based reachability check and loss/recovery handling.\n