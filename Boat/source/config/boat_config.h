/*
 Flytec V080 boat — R36 selected modular architecture.
 ESP32-S3-DevKitC-1 + HGLRC M100 PRO GPS + external EBYTE E22 UART LoRa.
 Exact DevKitC-1 variant, M100 PRO compass wiring, E22 UART RF compatibility
 and electrical pin map are NOT commissioned. All actuators stay disabled.
 No custom PCB, no second MCU, sonar remains unselected.
*/
#pragma once
#define BOAT_CONTROLLER_ESP32S3_DEVKITC1_CANDIDATE 1
#define BOAT_GNSS_HGLRC_M100_PRO_SELECTED 1
#define BOAT_BACKUP_E22_UART_ARCHITECTURE 1
#define BOAT_HARDWARE_COMMISSIONED 0
#define BOAT_ACTUATOR_POWER_PERMITTED 0
// GPIOs intentionally unassigned pending exact SKU, boot and header review.
