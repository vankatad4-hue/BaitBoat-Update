# R27 — Premade module GPIO recovery options and engineering decision matrix
Date 2026-11-21. No custom PCB, no second MCU, no direct motor/coil energization.

## Starting point
R26: six *provisional* accessible native GPIOs (4/5/6/7/17/26) versus nine for dedicated nRF SPI(3), CE+CSN(2), I2C(2), motor PWM(2). Exact v1.2 manufacturer pinout not verified. All numbers are a **feasibility analysis**, not a wiring assignment.

## Option A — I2C SPI bridge SC18IS602B
Commercial breakout/assembled adapter only; NXP SC18IS602B converts I2C to SPI with SPI clock up to 1.8 MHz, and four SPI slave selects; GPIO/interrupt options depend on breakout. NXP datasheet: https://www.nxp.com/docs/en/data-sheet/SC18IS602B.pdf
Resource budget: I2C(2) + motor PWM(2) + nRF CE(1) = 5 native GPIO; CSN driven by bridge hardware. DIR and requests through qualified I2C expander. nRF SPI throughput and library API must be rewritten for bridge transactions, and IRQ may need polling; bridge SPI latency/packet reliability are not verified. nRF24 normally supports SPI mode 0 and clock up to 10MHz, but bridge maximum 1.8MHz reduces command throughput; control packet latency must be measured. Verify breakout supply 3.3V, I2C address collision, bridge SS timing, nRF power and bus fault recovery. A module merely sold as SC18IS602B without exact breakout datasheet is NOT approved.

## Option B — Move PWM to commercially built I2C PWM module
PCA9685 off-the-shelf PWM module offers 16 outputs, typical PWM update range about 24–1526 Hz depending oscillator/prescale. Manufacturer datasheet: https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf
Resource budget: dedicated nRF SPI(3) + CE/CSN(2) + I2C(2) = 7 native GPIO, still **one short** of six. Moving CE/CSN to a qualified I2C GPIO expander can reduce native demand to 5, but nRF CE pulse width and CSN transaction sequencing through expander must be validated; safety/latency and MDD20A PWM frequency must be verified. This approach adds two I2C modules and a more complex control path, so it is not preferred without bench evidence.

## Option C — Manufacturer-confirmed additional pins / shared SX1262 SPI
If Heltec v1.2 official schematic confirms at least three more safe header GPIOs, native SPI is feasible. Alternatively if SX1262 SPI bus physically accessible with verified bus contention and CS handling, native demand falls to six, but no official v1.2 proof is available. Do NOT wire to onboard SX1262 solder pads based on v1.1 schematic alone.

## Option D — Change hardware topology
Different manufacturer-built controller with enough pins, or an off-the-shelf dual-radio integrated controller, only with explicit owner approval. Keep v1.2 selection until evidence rules it out.

## Preliminary choice
**Investigate Option A first**, not yet approve. It potentially fits within six provisional pins, avoids custom PCB/second MCU, and preserves two native motor PWM signals. Its bottleneck is bridge command throughput and a validated nRF24 transport driver. The independent safety interlock, reset default-off behavior, motor DIR expander, and actual v1.2 header mapping remain blocking.

## Bench acceptance
1. Manufacturer-confirmed v1.2 external header and boot-time behavior.
2. Exact ready-made SC18IS602B breakout available from a trusted supplier; full module pinout, power, address, CS, and IRQ verified.
3. Host test of bridge framing, then nRF24 packet throughput and latency at required radio link rate with realistic retries.
4. Fault injection: I2C stuck bus, bridge reset, nRF missing, stale CE, brownout. Independent STOP removes actuator power.
5. Motor PWM signal frequency and MDD20A direction switching, dead-time and failsafe verified with motors disconnected.
6. Physical bench review before any wiring release. No changes to official Remote VER028 / Boat VER014.
