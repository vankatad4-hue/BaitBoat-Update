
#ifndef JOYSTICK_CALIBRATION_UI_H
#define JOYSTICK_CALIBRATION_UI_H

#include <lvgl.h>

// Open the calibration screen.
// homeScreen is the existing main boat screen.
void joystickCalibrationUI_open(lv_obj_t *homeScreen);

// Close the calibration screen and return to the main screen.
void joystickCalibrationUI_close();

bool joystickCalibrationUI_isOpen();
#endif
