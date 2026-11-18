#ifndef DISPLAY_PORT_H
#define DISPLAY_PORT_H

// Hardware-independent contract between the application and the display board.
// The Waveshare-specific implementation belongs in display_port.cpp.

// Initialise the Waveshare LCD, touch controller and LVGL runtime exactly once.
// Return true only after LVGL is ready for UI object creation.
bool displayPort_begin();

// Service the display runtime only if the chosen Waveshare example requires
// the Arduino loop to do so. If its FreeRTOS task already owns LVGL scheduling,
// this function must remain empty.
void displayPort_loop();

// Report whether displayPort_begin() completed successfully.
bool displayPort_isReady();

// Report whether the Waveshare TF-card interface mounted successfully.
bool displayPort_sdReady();

// Switch the board backlight using the CH422G output configured by this project.
// This board configuration currently supports on/off, not variable PWM brightness.
bool displayPort_setBacklight(bool enabled);

#endif
