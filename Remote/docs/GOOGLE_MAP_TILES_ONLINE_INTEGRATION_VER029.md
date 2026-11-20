# Google online Map Tiles API integration design — ver029

**Research basis:** official Google Maps Platform documentation checked 2026-11-20. This is a design/specification, not an implementation.

## Suitable API
Use the Google Maps Platform **Map Tiles API 2D Tiles** for custom hardware. Google's overview explicitly lists custom hardware/navigation systems as a supported use case and offers 2D Roadmap, Satellite and Terrain tiles. This is not the Android Maps SDK.

Official documentation:
- Overview: https://developers.google.com/maps/documentation/tile/overview
- Quick reference: https://developers.google.com/maps/documentation/tile/quick-reference
- Create a session: https://developers.google.com/maps/documentation/tile/create-session
- 2D tile requests: https://developers.google.com/maps/documentation/tile/2d-tiles
- Policies: https://developers.google.com/maps/documentation/tile/policies
- Key setup: https://developers.google.com/maps/documentation/tile/get-api-key
- Billing/quota: https://developers.google.com/maps/documentation/tile/usage-and-billing

## Proposed client flow
1. Only attempt ONLINE when the user selected ONLINE or AUTO and Wi-Fi has internet reachability.
2. Use HTTPS/TLS certificate validation; never disable certificate verification.
3. Create a session token for the requested map type and options; handle session expiry by creating a new session.
4. Request the 2D tile for the current XYZ viewport using the session token and credential as documented.
5. Decode image data (PNG/JPEG as specified by the API response) into a transient PSRAM/RGB565 viewport buffer. Existing SD renderer remains a separate source path.
6. Show required Google and data-provider attribution in the map viewport; retain it in expanded map mode and make it legible.
7. Respect returned HTTP Cache-Control directives. Do not store Google content on SD, pre-fetch tiles, bulk download, export, stitch or repurpose the imagery. For the safest implementation, no persistent cache; transient display buffers only.
8. Handle HTTP errors, quota limits, session expiry and connection loss. Use bounded retry with backoff; never stall LVGL, telemetry, radio or safety/control tasks.
9. AUTO chooses online only while connectivity and provider/session requests are healthy; otherwise use a verified SD map. ONLINE without service shows an explicit unavailable status and overlays on neutral background. OFFLINE never makes provider requests.

## Credential / cost controls
- API key must not be hard-coded or committed to Git.
- Google strongly recommends restricting keys to the Map Tiles API. Application restriction options may be limited for a custom ESP32 device; assume a device-side key could be extracted and abused.
- Store the user-supplied key in protected device configuration only after flash-encryption/credential-storage design is reviewed. Do not claim the ESP32 key is secure against physical extraction.
- Configure a low daily quota and billing alerts before enabling the feature; each tile request is billable/quota-counted.
- Alternative safer credential architecture: a user-controlled authenticated gateway/proxy, subject to Google's service terms and security review; do not implement a proxy as a way to bypass tile caching or restrictions.
- EEA service terms may differ. Review applicable EEA terms because project owner/customer billing location may be in the EEA.

## API response / display constraints
- 2D tile requests require a session token created for the selected map type; direct tile requests without a valid session should be treated as errors.
- Use the viewport information service when needed to detect imagery/zoom coverage instead of assuming all areas support maximum zoom.
- Google content must be used as a map visualization only. No image analysis, fish detection, feature extraction, offline map conversion, or persistent SD map storage from Google tiles.
- Online imagery and offline maps are distinct renderer paths. Only the offline provider's tiles are converted into Flytec RGB565 files.

## Integration prerequisites
- Decide whether online map type (Roadmap vs Satellite) defaults to Satellite for fishing, with Roadmap optional.
- Select a PNG/JPEG decoder that fits the ESP32-S3 RAM/PSRAM and LVGL integration.
- Implement Wi-Fi internet reachability (not merely association to an access point).
- Implement transient tile viewport buffering and attribution UI.
- Provision credential, quota and billing settings manually; do not put secrets into repository or release ZIP.
- Test API key restrictions, TLS, session expiry, quota/403/429, slow responses, hotspot loss, memory fragmentation, map expansion, and offline fallback.

## Scope boundary
Online Google Maps imagery is not downloadable for offline use. The 1 km SD map area must use an independent provider/data source with explicit offline rights.
