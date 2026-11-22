#pragma once
// R21 candidate 3S pack measurement divider. Not signed/commissioned.
// 150k BAT_SW+ --> ADS1115 AIN0; 33k AIN0 --> GND; 100nF AIN0 --> GND.
// Power the divider from the same *switched* battery domain as the ADC.
// A permanently live divider can back-power an unpowered ADC through AIN0.
static constexpr double BOAT_BATTERY_DIVIDER_TOP_OHMS = 150000.0;
static constexpr double BOAT_BATTERY_DIVIDER_BOTTOM_OHMS = 33000.0;
static constexpr double BOAT_BATTERY_DIVIDER_RATIO =
    (BOAT_BATTERY_DIVIDER_TOP_OHMS + BOAT_BATTERY_DIVIDER_BOTTOM_OHMS) /
    BOAT_BATTERY_DIVIDER_BOTTOM_OHMS;
static constexpr double BOAT_BATTERY_ADC_VDD_VOLTS = 3.3;
inline double boatBatteryDividerInputVolts(double packVolts) {
 return packVolts / BOAT_BATTERY_DIVIDER_RATIO;
}
