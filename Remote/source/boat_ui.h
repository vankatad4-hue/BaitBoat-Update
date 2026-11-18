#ifndef BOAT_UI_H
#define BOAT_UI_H

#include <lvgl.h>
#include "boat_state.h"

// Initialise and create the main remote-control screen.
void createBoatScreen();

// Refresh the main screen using the latest shared boat state.
void updateBoatScreen(const BoatState &state);

// Expand or restore the map panel.
void toggleMapPanel();

// Expand or restore the sonar panel.
void toggleSonarPanel();

// Emergency-stop callback registered by the application.
typedef void (*StopCallback)();
void boatUI_setStopCallback(StopCallback callback);

#endif
