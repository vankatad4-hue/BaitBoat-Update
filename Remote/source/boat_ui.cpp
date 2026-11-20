
#include "boat_ui.h"
#include <Arduino.h>
#include "boat_state.h"
#include "joystick_calibration_ui.h"
#include "settings_ui.h"
#include "joystick.h"
#include "boat_radio.h"
#include "boat_commands.h"
#include "boat_telemetry.h"
#include "release_button_guard.h"
#include "mission_ui.h"
#include "map_renderer.h"
#include "map_source.h"
#include "map_manager_ui.h"
#include <math.h>
// --------------------------------------------------
// SCREEN DIMENSIONS
// --------------------------------------------------

static const int SCREEN_WIDTH  = 800;
static const int SCREEN_HEIGHT = 480;

static const int HEADER_HEIGHT = 46;
static const int FOOTER_HEIGHT = 100;

// --------------------------------------------------
// LVGL OBJECTS
// --------------------------------------------------

static lv_obj_t *mainScreen;       // Main screen object.
static lv_obj_t *headerPanel;      // Top status bar.
static lv_obj_t *contentPanel;     // Main display area.
static lv_obj_t *footerPanel;      // Bottom control bar.

static lv_obj_t *mapPanel;         // Map display panel.
static lv_obj_t *sonarPanel;       // Sonar display panel.

static lv_obj_t *connectionLabel;  // Radio status.
static lv_obj_t *batteryLabel;     // Battery information.
static lv_obj_t *modeLabel;        // Operating mode.

static lv_obj_t *mapTitle;         // Map panel title.
static lv_obj_t *gpsMapStatusLabel; // GPS fix / offline-map status.
static lv_obj_t *gpsCoordinatesLabel; // Latest boat latitude and longitude.
static lv_obj_t *gpsDetailsLabel;  // Satellite count, accuracy and heading.
static lv_obj_t *mapViewport;
static lv_obj_t *mapZoomLabel;
static lv_obj_t *mapSourceLabel;
static lv_obj_t *sonarTitle;       // Sonar panel title.

static lv_obj_t *stopButton;       // Emergency stop button.

static bool mapExpanded = false;   // Map full-screen state.
static bool sonarExpanded = false; // Sonar full-screen state.

static StopCallback stopCallback = nullptr;

// --------------------------------------------------
// COLOURS
// --------------------------------------------------

static const lv_color_t COLOR_BACKGROUND =
    lv_color_hex(0x081522);

static const lv_color_t COLOR_PANEL =
    lv_color_hex(0x10243A);

static const lv_color_t COLOR_TEXT =
    lv_color_hex(0xFFFFFF);

static const lv_color_t COLOR_RED =
    lv_color_hex(0xD91E2B);

static const lv_color_t COLOR_GREEN =
    lv_color_hex(0x20C979);

// --------------------------------------------------
// HELPER: CREATE A LABEL
// --------------------------------------------------

static lv_obj_t *createLabel(
    lv_obj_t *parent,
    const char *text,
    int x,
    int y)
{
    // Create a new LVGL label.
    lv_obj_t *label = lv_label_create(parent);

    // Set the displayed text.
    lv_label_set_text(label, text);

    // Set the label position.
    lv_obj_set_pos(label, x, y);

    // Set the text colour.
    lv_obj_set_style_text_color(
        label,
        COLOR_TEXT,
        LV_PART_MAIN
    );

    // Return the created label.
    return label;
}

// --------------------------------------------------
// HELPER: CREATE A PANEL
// --------------------------------------------------

static lv_obj_t *createPanel(
    lv_obj_t *parent,
    int x,
    int y,
    int width,
    int height)
{
    // Create a new panel object.
    lv_obj_t *panel = lv_obj_create(parent);

    // Set the panel position.
    lv_obj_set_pos(panel, x, y);

    // Set the panel dimensions.
    lv_obj_set_size(panel, width, height);

    // Set the background colour.
    lv_obj_set_style_bg_color(
        panel,
        COLOR_PANEL,
        LV_PART_MAIN
    );

    // Remove the border.
    lv_obj_set_style_border_width(
        panel,
        0,
        LV_PART_MAIN
    );

    // Set rounded corners.
    lv_obj_set_style_radius(
        panel,
        8,
        LV_PART_MAIN
    );

    // Remove internal padding.
    lv_obj_set_style_pad_all(
        panel,
        0,
        LV_PART_MAIN
    );

    // Add a restrained blue outline for panel separation.
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x2366A4), LV_PART_MAIN);

    // Return the panel.
    return panel;
}

// Apply the shared high-contrast touch-button style.
static void styleTouchButton(lv_obj_t *button, lv_color_t fill)
{
    lv_obj_set_style_bg_color(button, fill, LV_PART_MAIN);
    lv_obj_set_style_text_color(button, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_radius(button, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(0x347FC0), LV_PART_MAIN);
}

// --------------------------------------------------
// MAP PANEL TOUCH EVENT
// --------------------------------------------------

static void mapTouchEvent(lv_event_t *event)
{
    // Only react to a completed touch.
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
    {
        // Expand or restore the map.
        toggleMapPanel();
    }
}

// --------------------------------------------------
// SONAR PANEL TOUCH EVENT
// --------------------------------------------------

static void sonarTouchEvent(lv_event_t *event)
{
    // Only react to a completed touch.
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
    {
        // Expand or restore the sonar.
        toggleSonarPanel();
    }
}

// --------------------------------------------------
// EMERGENCY STOP EVENT
// --------------------------------------------------

static void stopTouchEvent(lv_event_t *event)
{
    // Check whether the STOP button was pressed.
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
    {
        // Stop both motors in the displayed state.
        boatState.motorLeftActive = false;
        boatState.motorRightActive = false;

        // Mark the emergency stop as active.
        boatState.emergencyStop = true;
        boatState.emergencyStopConfirmed = false;

        // Call the registered safety handler.
        if (stopCallback != nullptr)
        {
            stopCallback();
        }

        // Refresh the display.
        updateBoatScreen(boatState);
    }
}

// --------------------------------------------------
// CREATE HEADER
// --------------------------------------------------

static void createHeader()
{
    // Create the top status panel.
    headerPanel = createPanel(
        mainScreen,
        0,
        0,
        SCREEN_WIDTH,
        HEADER_HEIGHT
    );

    // Display radio connection status.
    connectionLabel = createLabel(
        headerPanel,
        "РАДИО: --",
        12,
        17
    );

    // Display boat battery information.
    batteryLabel = createLabel(
        headerPanel,
        "БАТЕРИЯ: --%",
        280,
        17
    );

    // Display operating mode.
    modeLabel = createLabel(
        headerPanel,
        "РЕЖИМ: РЪЧЕН",
        535,
        17
    );

    // Add a gear button to open remote settings.
    lv_obj_t *settingsButton = lv_button_create(headerPanel);
    styleTouchButton(settingsButton, lv_color_hex(0x174F92));
    lv_obj_set_size(settingsButton, 100, 36);
    lv_obj_align(settingsButton, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_t *settingsText = lv_label_create(settingsButton);
    lv_label_set_text(settingsText, "НАСТРОЙКИ");
    lv_obj_center(settingsText);
    lv_obj_add_event_cb(settingsButton, [](lv_event_t *event) {
        if (lv_event_get_code(event) == LV_EVENT_CLICKED)
            settingsUI_open(mainScreen);
    }, LV_EVENT_CLICKED, nullptr);
}

// --------------------------------------------------
// CREATE MAP PANEL
// --------------------------------------------------

static void mapZoomInEvent(lv_event_t *event){if(lv_event_get_code(event)==LV_EVENT_CLICKED){mapRenderer_zoomIn();if(mapZoomLabel){char z[8];snprintf(z,sizeof(z),"Z%u",(unsigned)mapRenderer_getZoom());lv_label_set_text(mapZoomLabel,z);}}}
static void mapZoomOutEvent(lv_event_t *event){if(lv_event_get_code(event)==LV_EVENT_CLICKED){mapRenderer_zoomOut();if(mapZoomLabel){char z[8];snprintf(z,sizeof(z),"Z%u",(unsigned)mapRenderer_getZoom());lv_label_set_text(mapZoomLabel,z);}}}
static void mapSourceCycleEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    const MapSourceMode mode = mapSource_nextMode();
    if (mapSourceLabel) lv_label_set_text(mapSourceLabel, mapSource_modeLabel(mode));
    updateBoatScreen(boatState);
}
static void mapManagerOpenEvent(lv_event_t *event){if(lv_event_get_code(event)!=LV_EVENT_CLICKED)return;lv_obj_add_flag(headerPanel,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(contentPanel,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(footerPanel,LV_OBJ_FLAG_HIDDEN);mapManagerUI_open(mainScreen,boatState.latitude,boatState.longitude,boatState.gpsValid,[]{lv_obj_remove_flag(headerPanel,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(contentPanel,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(footerPanel,LV_OBJ_FLAG_HIDDEN);});}\nstatic void createMapPanel()
{
    mapPanel=createPanel(contentPanel,5,5,385,315);
    mapTitle=createLabel(mapPanel,"КАРТА",10,8);
    lv_obj_t *sourceButton=lv_button_create(mapPanel);styleTouchButton(sourceButton, lv_color_hex(0x174F92));
    lv_obj_set_size(sourceButton,86,28);lv_obj_set_pos(sourceButton,285,4);
    mapSourceLabel=lv_label_create(sourceButton);lv_label_set_text(mapSourceLabel,mapSource_modeLabel(mapSource_getMode()));lv_obj_center(mapSourceLabel);
    lv_obj_add_event_cb(sourceButton,mapSourceCycleEvent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *manager=lv_button_create(mapPanel);styleTouchButton(manager, lv_color_hex(0x174F92));lv_obj_set_size(manager,92,28);lv_obj_set_pos(manager,180,4);lv_label_set_text(lv_label_create(manager),"УПРАВЛЕНИЕ");lv_obj_center(lv_obj_get_child(manager,0));lv_obj_add_event_cb(manager,mapManagerOpenEvent,LV_EVENT_CLICKED,nullptr);
    gpsCoordinatesLabel=createLabel(mapPanel,"ШИР: --.------  ДЪЛ: --.------",10,31);
    gpsMapStatusLabel=createLabel(mapPanel,"GPS: НЯМА FIX | SD: ПРОВЕРКА",10,53);
    lv_obj_set_width(gpsMapStatusLabel,365);lv_label_set_long_mode(gpsMapStatusLabel,LV_LABEL_LONG_DOT);
    mapViewport=lv_obj_create(mapPanel);
    lv_obj_set_pos(mapViewport,8,78);lv_obj_set_size(mapViewport,369,190);
    lv_obj_set_style_border_width(mapViewport,0,LV_PART_MAIN);lv_obj_set_style_pad_all(mapViewport,0,LV_PART_MAIN);
    lv_obj_set_style_bg_color(mapViewport,lv_color_hex(0x263746),LV_PART_MAIN);lv_obj_set_style_clip_corner(mapViewport,true,LV_PART_MAIN);
    lv_obj_clear_flag(mapViewport,LV_OBJ_FLAG_SCROLLABLE);mapRenderer_create(mapViewport);
    lv_obj_t *zoomOut=lv_button_create(mapPanel);styleTouchButton(zoomOut, lv_color_hex(0x174F92));lv_obj_set_size(zoomOut,42,32);lv_obj_set_pos(zoomOut,8,270);
    lv_label_set_text(lv_label_create(zoomOut),"-");lv_obj_center(lv_obj_get_child(zoomOut,0));lv_obj_add_event_cb(zoomOut,mapZoomOutEvent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *zoomIn=lv_button_create(mapPanel);styleTouchButton(zoomIn, lv_color_hex(0x174F92));lv_obj_set_size(zoomIn,42,32);lv_obj_set_pos(zoomIn,54,270);
    lv_label_set_text(lv_label_create(zoomIn),"+");lv_obj_center(lv_obj_get_child(zoomIn,0));lv_obj_add_event_cb(zoomIn,mapZoomInEvent,LV_EVENT_CLICKED,nullptr);
    mapZoomLabel=createLabel(mapPanel,"Z15",104,278);gpsDetailsLabel=createLabel(mapPanel,"SAT: --  ACC: --m  КУРС: --",160,278);
    lv_obj_add_flag(mapPanel,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(mapPanel,mapTouchEvent,LV_EVENT_CLICKED,nullptr);
}

// --------------------------------------------------
// CREATE SONAR PANEL
// --------------------------------------------------

// Static visual-only echogram prototype. Every arch is marked DEMO; no sonar
// measurements are fabricated or connected to boat telemetry.
static lv_obj_t *sonarPlot = nullptr;
static lv_obj_t *sonarStatus = nullptr;

static void sonarLine(lv_obj_t *parent, const lv_point_t *pts, uint16_t n,
                      lv_color_t color, uint8_t width)
{
    lv_obj_t *line = lv_line_create(parent);
    lv_line_set_points(line, pts, n);
    lv_obj_set_style_line_color(line, color, LV_PART_MAIN);
    lv_obj_set_style_line_width(line, width, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);
}

static void createSonarPanel()
{
    sonarPanel = createPanel(contentPanel, 400, 5, 385, 315);
    sonarTitle = createLabel(sonarPanel, "СОНАР • CHIRP", 12, 8);
    createLabel(sonarPanel, "ДЪЛБОЧИНА  --.- m", 12, 30);

    sonarPlot = lv_obj_create(sonarPanel);
    lv_obj_set_pos(sonarPlot, 42, 52);
    lv_obj_set_size(sonarPlot, 330, 210);
    lv_obj_set_style_bg_color(sonarPlot, lv_color_hex(0x06101B), LV_PART_MAIN);
    lv_obj_set_style_border_width(sonarPlot, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(sonarPlot, lv_color_hex(0x28618A), LV_PART_MAIN);
    lv_obj_set_style_pad_all(sonarPlot, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sonarPlot, LV_OBJ_FLAG_SCROLLABLE);

    // Horizontal depth guides.
    static const lv_point_t h0[]={{0,0},{329,0}}, h1[]={{0,52},{329,52}},
      h2[]={{0,104},{329,104}}, h3[]={{0,156},{329,156}}, h4[]={{0,208},{329,208}};
    const lv_color_t grid=lv_color_hex(0x244258);
    sonarLine(sonarPlot,h0,2,grid,1); sonarLine(sonarPlot,h1,2,grid,1);
    sonarLine(sonarPlot,h2,2,grid,1); sonarLine(sonarPlot,h3,2,grid,1);
    sonarLine(sonarPlot,h4,2,grid,1);
    static const lv_point_t v0[]={{0,0},{0,208}},v1[]={{82,0},{82,208}},
      v2[]={{164,0},{164,208}},v3[]={{246,0},{246,208}},v4[]={{329,0},{329,208}};
    sonarLine(sonarPlot,v0,2,grid,1);sonarLine(sonarPlot,v1,2,grid,1);
    sonarLine(sonarPlot,v2,2,grid,1);sonarLine(sonarPlot,v3,2,grid,1);
    sonarLine(sonarPlot,v4,2,grid,1);

    // Example fish-arch traces, deliberately illustrative only.
    static const lv_point_t archA[]={{34,144},{62,120},{92,94},{124,82},{153,94},{184,120},{212,145}};
    static const lv_point_t archB[]={{168,188},{189,165},{212,142},{236,128},{258,142},{279,165},{298,188}};
    static const lv_point_t bottom[]={{8,198},{60,195},{110,200},{160,194},{210,199},{260,196},{320,201}};
    sonarLine(sonarPlot,archA,7,lv_color_hex(0x35E3B0),2);
    sonarLine(sonarPlot,archB,7,lv_color_hex(0xF5C84C),2);
    sonarLine(sonarPlot,bottom,7,lv_color_hex(0xE67E45),2);

    createLabel(sonarPanel,"0",8,52); createLabel(sonarPanel,"5",8,103);
    createLabel(sonarPanel,"10",4,155); createLabel(sonarPanel,"15",4,207);
    createLabel(sonarPanel,"DEMO • НЕ Е РЕАЛЕН СОНАР",12,270);
    sonarStatus=createLabel(sonarPanel,"МОДУЛ / ДАННИ: НЕ Е СВЪРЗАН",12,289);

    lv_obj_add_flag(sonarPanel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sonarPanel, sonarTouchEvent, LV_EVENT_CLICKED, nullptr);
}

// Touchscreen mode controls share the same command path as physical keys.
static void touchManualEvent(lv_event_t *event)
{
    if (lv_event_get_code(event)==LV_EVENT_CLICKED) boatCommands_setMode(MODE_MANUAL);
}
static void touchAutopilotEvent(lv_event_t *event)
{
    if (lv_event_get_code(event)==LV_EVENT_CLICKED) boatCommands_setMode(MODE_AUTOPILOT);
}
static void touchReturnEvent(lv_event_t *event)
{
    if (lv_event_get_code(event)==LV_EVENT_CLICKED) boatCommands_setMode(MODE_RETURN_HOME);
}
static void openMissionEvent(lv_event_t *event)
{
    if (lv_event_get_code(event)==LV_EVENT_CLICKED) missionUI_open(mainScreen);
}

// Open the joystick calibration screen.
static void openJoystickCalibrationEvent(lv_event_t *event)
{
    // Respond only to a completed button click.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Open calibration and remember the main screen.
    joystickCalibrationUI_open(mainScreen);
}


// --------------------------------------------------
// CREATE FOOTER
// --------------------------------------------------

static void createFooter()
{
    // Create the bottom control panel.
    footerPanel = createPanel(
        mainScreen,
        0,
        SCREEN_HEIGHT - FOOTER_HEIGHT,
        SCREEN_WIDTH,
        FOOTER_HEIGHT
    );

    // Create the emergency STOP button.
    stopButton = lv_button_create(footerPanel);
    styleTouchButton(stopButton, COLOR_RED);

    // Set the STOP button size.
    lv_obj_set_size(stopButton, 210, 46);

    // Position the STOP button on the right.
    lv_obj_set_pos(stopButton, 570, 47);

    // Set the STOP button background colour.
    lv_obj_set_style_bg_color(
        stopButton,
        COLOR_RED,
        LV_PART_MAIN
    );

    // Create the STOP button text.
    lv_obj_t *stopLabel = lv_label_create(stopButton);

    // Set the button text.
    lv_label_set_text(stopLabel, "СТОП");

    // Centre the text.
    lv_obj_center(stopLabel);

    // Register the emergency stop event.
    lv_obj_add_event_cb(
        stopButton,
        stopTouchEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    // Compact touchscreen mode controls. Boat-side command policy remains authoritative.
    lv_obj_t *manualButton = lv_button_create(footerPanel);
    styleTouchButton(manualButton, lv_color_hex(0x174F92));
    lv_obj_set_size(manualButton, 105, 44); lv_obj_set_pos(manualButton, 10, 47);
    lv_label_set_text(lv_label_create(manualButton), "РЪЧНО"); lv_obj_center(lv_obj_get_child(manualButton,0));
    lv_obj_add_event_cb(manualButton,touchManualEvent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *autoButton = lv_button_create(footerPanel);
    styleTouchButton(autoButton, lv_color_hex(0x174F92));
    lv_obj_set_size(autoButton, 118, 44); lv_obj_set_pos(autoButton, 120, 47);
    lv_label_set_text(lv_label_create(autoButton), "АВТОПИЛОТ"); lv_obj_center(lv_obj_get_child(autoButton,0));
    lv_obj_add_event_cb(autoButton,touchAutopilotEvent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *returnButton = lv_button_create(footerPanel);
    styleTouchButton(returnButton, lv_color_hex(0x174F92));
    lv_obj_set_size(returnButton, 110, 44); lv_obj_set_pos(returnButton, 248, 47);
    lv_label_set_text(lv_label_create(returnButton), "КЪМ ДОМА"); lv_obj_center(lv_obj_get_child(returnButton,0));
    lv_obj_add_event_cb(returnButton,touchReturnEvent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *missionButton = lv_button_create(footerPanel);
    styleTouchButton(missionButton, lv_color_hex(0x174F92));
    lv_obj_set_size(missionButton, 100, 44); lv_obj_set_pos(missionButton, 365, 47);
    lv_label_set_text(lv_label_create(missionButton), "МИСИЯ"); lv_obj_center(lv_obj_get_child(missionButton,0));
    lv_obj_add_event_cb(missionButton,openMissionEvent,LV_EVENT_CLICKED,nullptr);

    // Show live input validity and calibration status, not a generic reminder.
    inputStatusLabel = createLabel(footerPanel, "СТИКОВЕ: НЕСЪПРОВЕРЕНИ", 15, 4);
    lv_obj_set_width(inputStatusLabel, 380);
    lv_label_set_long_mode(inputStatusLabel, LV_LABEL_LONG_DOT);
    safetyStatusLabel = createLabel(footerPanel, "STOP: НЕИЗВЕСТНО", 410, 4);
    lv_obj_set_width(safetyStatusLabel, 380);
    lv_label_set_long_mode(safetyStatusLabel, LV_LABEL_LONG_DOT);
    // Create a button for joystick calibration.
lv_obj_t *sticksButton = lv_button_create(footerPanel);
styleTouchButton(sticksButton, lv_color_hex(0x174F92));

// Set its size.
lv_obj_set_size(sticksButton, 80, 42);

// Position in the footer row, before STOP.
lv_obj_set_pos(sticksButton, 470, 47);

// Create the button label.
lv_obj_t *sticksLabel = lv_label_create(sticksButton);

// Set its text.
lv_label_set_text(sticksLabel, "КАЛИБР.");

// Centre the text.
lv_obj_center(sticksLabel);

// Register the button event.
lv_obj_add_event_cb(
    sticksButton,
    openJoystickCalibrationEvent,
    LV_EVENT_CLICKED,
    nullptr
);
}

// --------------------------------------------------
// INITIALISE THE COMPLETE SCREEN
// --------------------------------------------------

void createBoatScreen()
{
    mapSource_begin();

    // Get the active LVGL screen.
    mainScreen = lv_screen_active();

    // Set the main background colour.
    lv_obj_set_style_bg_color(
        mainScreen,
        COLOR_BACKGROUND,
        LV_PART_MAIN
    );

    // Remove the main screen scrolling behaviour.
    lv_obj_remove_flag(
        mainScreen,
        LV_OBJ_FLAG_SCROLLABLE
    );

    // Create the top status bar.
    createHeader();

    // Create the main content container.
    contentPanel = createPanel(
        mainScreen,
        5,
        HEADER_HEIGHT + 3,
        790,
        325
    );

    // Create the map panel.
    createMapPanel();

    // Create the sonar panel.
    createSonarPanel();

    // Create the bottom control bar.
    createFooter();

    // Display initial boat information.
    updateBoatScreen(boatState);
}

// --------------------------------------------------
// EXPAND OR RESTORE MAP
// --------------------------------------------------

void toggleMapPanel()
{
    // Switch the expanded state.
    mapExpanded = !mapExpanded;

    // Hide sonar while map is expanded.
    if (mapExpanded)
    {
        sonarExpanded = false;

        // Hide the sonar panel.
        lv_obj_add_flag(
            sonarPanel,
            LV_OBJ_FLAG_HIDDEN
        );

        // Expand map to fill the content area.
        lv_obj_set_pos(mapPanel, 5, 5);

        lv_obj_set_size(mapPanel, 780, 315);
        lv_obj_set_size(mapViewport, 760, 220);
    }
    else
    {
        // Restore the original map dimensions.
        lv_obj_set_pos(mapPanel, 5, 5);

        lv_obj_set_size(mapPanel, 385, 315);
        lv_obj_set_size(mapViewport, 369, 190);

        // Show sonar again.
        lv_obj_remove_flag(
            sonarPanel,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}

// --------------------------------------------------
// EXPAND OR RESTORE SONAR
// --------------------------------------------------

void toggleSonarPanel()
{
    // Switch the expanded state.
    sonarExpanded = !sonarExpanded;

    // Hide map while sonar is expanded.
    if (sonarExpanded)
    {
        mapExpanded = false;

        // Hide the map panel.
        lv_obj_add_flag(
            mapPanel,
            LV_OBJ_FLAG_HIDDEN
        );

        // Expand sonar to fill the content area.
        lv_obj_set_pos(sonarPanel, 5, 5);

        lv_obj_set_size(sonarPanel, 780, 315);
    }
    else
    {
        // Restore the original sonar dimensions.
        lv_obj_set_pos(sonarPanel, 400, 5);

        lv_obj_set_size(sonarPanel, 385, 315);

        // Show map again.
        lv_obj_remove_flag(
            mapPanel,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}

// --------------------------------------------------
// UPDATE DISPLAYED BOAT INFORMATION
// --------------------------------------------------

static void updateMapMissionMarkers(const BoatState &state)
{
    MapRendererWaypoint points[16]={};
    for(uint8_t i=0;i<16;i++){float lat=0,lon=0;bool bait=false,hook=false;points[i].valid=missionUI_getWaypoint(i,&lat,&lon,&bait,&hook);points[i].lat=lat;points[i].lon=lon;points[i].bait=bait;points[i].hook=hook;}
    float hlat=0,hlon=0;bool hv=missionUI_getHome(&hlat,&hlon);
    bool fresh=state.gpsValid&&(uint32_t)(millis()-state.gpsLastUpdateMs)<=2000U;
    mapRenderer_update(fresh,state.latitude,state.longitude,state.heading,hv,hlat,hlon,points,(uint8_t)missionUI_getSelectedIndex(),state.mode==MODE_RETURN_HOME);
    if(mapZoomLabel){char z[8];snprintf(z,sizeof(z),"Z%u",(unsigned)mapRenderer_getZoom());lv_label_set_text(mapZoomLabel,z);}
}

void updateBoatScreen(const BoatState &state)
{
    // Update the GPS coordinate display only when telemetry reports a valid fix.
    if (state.gpsValid)
    {
        char coordinateText[80];
        snprintf(
            coordinateText,
            sizeof(coordinateText),
            "ШИР: %.6f   ДЪЛ: %.6f",
            state.latitude,
            state.longitude
        );
        lv_label_set_text(gpsCoordinatesLabel, coordinateText);

        char mapStatusText[96];
        const char *mapStorageText = state.offlineMapReady ? "SD MAP READY" : (state.sdCardReady ? "NO SD MAP" : "SD NOT READY");
        const MapSourceMode sourceMode = mapSource_getMode();
        const char *sourceText = (sourceMode == MAP_SOURCE_ONLINE && !mapSource_onlineAvailable()) ? "ONLINE UNAVAILABLE - SD FALLBACK" : (sourceMode == MAP_SOURCE_AUTO ? "AUTO / SD" : "OFFLINE / SD");
        snprintf(mapStatusText, sizeof(mapStatusText), "GPS: FIX  |  %s  |  %s", mapStorageText, sourceText);
        lv_label_set_text(gpsMapStatusLabel, mapStatusText);

        char gpsDetailsText[80];
        snprintf(
            gpsDetailsText,
            sizeof(gpsDetailsText),
            "SAT: %u   ACC: %.1f m   КУРС: %.0f°",
            static_cast<unsigned>(state.gpsSatellites),
            state.gpsAccuracyMeters,
            state.heading
        );
        lv_label_set_text(gpsDetailsLabel, gpsDetailsText);
    }
    else
    {
        lv_label_set_text(gpsCoordinatesLabel, "LAT: --.------   LON: --.------");
        const char *mapStorageText = state.offlineMapReady ? "SD MAP READY" : (state.sdCardReady ? "NO SD MAP" : "SD NOT READY");
        const MapSourceMode sourceMode = mapSource_getMode();
        const char *sourceText = (sourceMode == MAP_SOURCE_ONLINE && !mapSource_onlineAvailable()) ? "ONLINE UNAVAILABLE - SD FALLBACK" : (sourceMode == MAP_SOURCE_AUTO ? "AUTO / SD" : "OFFLINE / SD");
        char mapStatusText[120];
        snprintf(mapStatusText, sizeof(mapStatusText), "GPS: НЯМА FIX  |  %s  |  %s", mapStorageText, sourceText);
        lv_label_set_text(gpsMapStatusLabel, mapStatusText);
        lv_label_set_text(gpsDetailsLabel, "SAT: --   ACC: -- m   КУРС: --°");
    }

    missionUI_initStorage();
    updateMapMissionMarkers(state);

    // Show selected link and failover state.
    const char *radioText = "РАДИО: НЯМА";
    switch (state.radioLinkState)
    {
        case RADIO_UI_PRIMARY: radioText = "РАДИО: NRF24"; break;
        case RADIO_UI_DEGRADED: radioText = "РАДИО: СЛАБА ВРЪЗКА"; break;
        case RADIO_UI_BACKUP: radioText = "РАДИО: РЕЗЕРВНО LORA"; break;
        case RADIO_UI_NO_LINK: radioText = "РАДИО: НЯМА ВРЪЗКА"; break;
        case RADIO_UI_RECOVERY: radioText = "РАДИО: ВЪЗСТАНОВЯВА"; break;
        default: radioText = "РАДИО: ИЗКЛЮЧЕНО"; break;
    }
    lv_label_set_text(connectionLabel, radioText);

    // Do not present a fabricated battery percentage before fresh telemetry.
    char batteryText[32];
    if (telemetry.batteryPercentValid && (uint32_t)(millis()-telemetry.batteryPercentReceivedAtMs) <= 1500U)
        snprintf(batteryText, sizeof(batteryText), "БАТЕРИЯ: %d%%", telemetry.batteryPercent);
    else
        snprintf(batteryText, sizeof(batteryText), "БАТЕРИЯ: --");

    lv_label_set_text(
        batteryLabel,
        batteryText
    );

    // Update operating mode.
    switch (state.mode)
    {
        case MODE_MANUAL:
            lv_label_set_text(
                modeLabel,
                "MODE: MANUAL"
            );
            break;

        case MODE_AUTOPILOT:
            lv_label_set_text(
                modeLabel,
                "РЕЖИМ: АВТОПИЛОТ"
            );
            break;

        case MODE_RETURN_HOME:
            lv_label_set_text(
                modeLabel,
                "РЕЖИМ: КЪМ ДОМА"
            );
            break;
    }

    // Refresh the live input and safety status footer.
    const JoystickSnapshot input = joystick_getSnapshot();
    char inputText[112];
    if (!input.readingsValid) snprintf(inputText,sizeof(inputText),"СТИКОВЕ: ADC ГРЕШКА / НЯМА ДАННИ");
    else if (!input.calibrationValid) snprintf(inputText,sizeof(inputText),"СТИКОВЕ: КАЛИБРИРАЙ  RAW %d/%d %d/%d",input.raw.leftX,input.raw.leftY,input.raw.rightX,input.raw.rightY);
    else snprintf(inputText,sizeof(inputText),"СТИКОВЕ L %d,%d  R %d,%d  %s",input.values.leftX,input.values.leftY,input.values.rightX,input.values.rightY,input.neutral?"НЕУТРАЛНИ":"АКТИВНИ");
    if (inputStatusLabel) lv_label_set_text(inputStatusLabel,inputText);
    char safetyText[128];
    const char *stopText = state.emergencyStop ? (state.emergencyStopConfirmed ? "ПОТВЪРДИ" : "ЗАКЛЮЧЕН?") : (state.emergencyStopConfirmed ? "ИЗЧИСТЕН" : "UNKNOWN");
    const char *healthText = state.failsafeActive ? "АВАР. РЕЖИМ" : (state.lowBattery ? "НИСКА БАТ." : (state.actuatorSafetyReady ? "ГОТОВО" : "БЛОКИРАНО"));
    snprintf(safetyText,sizeof(safetyText),"STOP:%s %s ACK:%u",stopText,healthText,(unsigned)boatRadio_pendingCommandAcks());
    if (safetyStatusLabel) lv_label_set_text(safetyStatusLabel,safetyText);

    // Change STOP button text when emergency stop is active.
    if (state.emergencyStop)
    {
        lv_label_set_text(
            lv_obj_get_child(stopButton, 0),
            "СТОП АКТИВЕН"
        );
    }
    else
    {
        lv_label_set_text(
            lv_obj_get_child(stopButton, 0),
            "STOP"
        );
    }
}

// --------------------------------------------------
// REGISTER EMERGENCY STOP CALLBACK
// --------------------------------------------------

void boatUI_setStopCallback(StopCallback callback)
{
    // Store the safety callback.
    stopCallback = callback;
}
