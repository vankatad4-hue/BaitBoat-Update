> **CURRENT R30H-10 GPIO PROFILE (2026-11-22):** The [joint GPIO resource plan](R30H_10_GPIO_RESOURCE_EVALUATION.md) is preliminary and read-only. Boat has candidate peripheral pins with RF audit flags=0; Remote nRF CE/CSN in `Remote/source/radio_hw_config.h` are now **-1/-1**, not former GPIO6/15. Remote SPI GPIO11/12/13 is currently shared with TF; actual SD CS via CH422G EXIO4 needs reviewed arbitration. Both compile-only PIO targets and the upload-denial scripts remain primary; **no upload/physical radio/motor release is authorized**. On the recent GPIO commit, Boat PIO job passed [#45066383457](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383457), while Remote PIO had not finished at this review. All 18 R30H-10 physical gates remain unsigned. Earlier successes below refer to earlier source commits.

# Flytec V080 — VS Code + PlatformIO primary workflow (R30H-10)

**Decision: 2026-11-22.** VS Code with PlatformIO is the **primary IDE, dependency manager and compile/CI path** for both Boat and Remote from this checkpoint onward. Arduino CLI/IDE stays as a **secondary compatibility regression**, not the build that determines the development workflow. This is a *development-tool promotion only*, **not** permission to flash, energize radios, connect actuators or ship firmware.

## Verified starting point

- [PlatformIO CI #45063024581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024581) reports **SUCCESS for Boat and Remote full targets**, with no upload.
- Boat environment: `Boat/platformio.ini`, `boat_n32r16v_diagnostic`, Espressif ESP32-S3-DevKitC-1 N32R16V, 32MB OPI flash / 16MB OPI PSRAM.
- Remote environment: `Remote/platformio.ini`, `remote_waveshare_43_diagnostic`, Waveshare ESP32-S3-Touch-LCD-4.3 **Standard**, LVGL 9.6.0, 16MB QIO flash / OPI PSRAM.
- Pinned PlatformIO platform: `espressif32 @ 8.0.0` (Arduino-ESP32 core 3.3.12); older Arduino CLI target uses core 3.0.2 as a comparison. A build success does **not** demonstrate board memory configuration or wiring on a purchased device.
- Both projects intentionally reuse the **current** source tree, without duplicating the application: Boat's guarded adapter includes its nested sketch; Remote's root `.ino` is compiled exactly once.

## First-party development steps

1. Install VS Code and the **PlatformIO IDE** extension.
2. Open `Flytec_V080.code-workspace` in repository root.
3. Select Boat or Remote project separately in PlatformIO Project Tasks.
4. Use **Build**, or execute these repository-root commands from a VS Code terminal:

```powershell
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

5. Review compiler warnings, artifact provenance and [primary PlatformIO Actions workflow](../.github/workflows/platformio-migration-build.yml). Run portable host tests and safety preflights on every functional change. Use Arduino CLI builds only as **secondary regression** when the change affects shared C++/library compatibility.
6. Remote Bulgarian font resources are generated from a locally licensed installed font using `Remote/tools/generate_bg_font.sh` (Linux/WSL dependencies in [original migration guide](PLATFORMIO_VSCODE_MIGRATION_R1.md)); never commit third-party font binaries.

The primary CI also runs [R30H-10 source-lock preflight](../Boat/tools/verify_r30h10_physical_lock.py), checking that all 18 hardware gates remain unsigned and that both PlatformIO upload-denial mechanisms and RF/marker pins stay fail-closed. A green preflight is **only** a source policy check; it cannot replace hardware measurements.

## HARD SAFETY BOUNDARY — applies to both projects

- **Upload/flash remains denied** by `upload_protocol = custom` plus `tools/pio_deny_upload.py`. No live motor, solenoid, RF, waypoint mission or RTH commissioning is authorized by a successful build.
- Physical GPIO audit macros, marker GPIO=-1, RF interop/airtime and independent actuator safety interlock gates remain **0/unsigned**. No synthetic PASS; no default pin reassignment.
- Remote **GPIO6** proposed nRF CE vs SD dummy CS and Sensor AD is a known actual software pin collision. Remote **GPIO15** proposed CSN vs Waveshare onboard RS485 TXD is a physical schematic conflict needing inspection. Both block Remote radio bring-up.
- Boat selected architecture: **one common fused 3S battery bus** (no independently isolated motor/solenoid supply). For RF-only testing, disconnect **both** MDD20A VM and EKM014 VH. Logic-only power is not permission to connect either.
- **Independent hardware STOP** (not dependent on firmware responsiveness or authenticated radio availability), verified current-zero, non-retriggerable solenoid pulse limit and physical watchdog acceptance are separate unresolved gates; compile success/host simulation are insufficient.
- GPS/PPS, both real RF paths, GPIO logic levels and analyzer waveforms remain untested until the exact purchased revisions are measured. Follow [R30H-10 physical acceptance](../Boat/docs/BOAT_R30H_10_HARDWARE_ACCEPTANCE.md).
- Official markers stay **Boat VER014 / Remote VER028**; firmware release/physical acceptance requires separate signoff.

## Source-of-truth order

For new changes: (1) `Boat/platformio.ini` and `Remote/platformio.ini`, (2) PlatformIO CI logs and host safety tests, (3) secondary Arduino regression, (4) physical evidence sheets. The [R1 migration guide](PLATFORMIO_VSCODE_MIGRATION_R1.md) is historical for how parallel builds were introduced; any claims there that PlatformIO is still experimental or Arduino CLI is still the primary *development* build are superseded by this decision. Nothing supersedes physical hardware signoffs.
