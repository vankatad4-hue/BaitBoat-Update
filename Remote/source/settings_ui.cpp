#include "settings_ui.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <cstring>
#include "display_port.h"
#include "joystick.h"
#include "joystick_calibration_ui.h"

// Persistent remote preferences are stored in the ESP32 NVS partition.
static Preferences settingsPrefs;
static lv_obj_t *settingsScreen = nullptr;
static lv_obj_t *returnToScreen = nullptr;
static lv_obj_t *ssidArea = nullptr;
static lv_obj_t *passwordArea = nullptr;
static lv_obj_t *keyboard = nullptr;
static lv_obj_t *wifiStatus = nullptr;
static lv_obj_t *scanResults = nullptr;
static lv_obj_t *brightnessSwitch = nullptr;
static lv_obj_t *themeSwitch = nullptr;
static lv_obj_t *timeoutLabel = nullptr;
static lv_obj_t *settingsContent = nullptr;
static lv_timer_t *wifiScanTimer = nullptr;
static lv_timer_t *wifiConnectTimer = nullptr;
static uint32_t wifiConnectStartedAt = 0;
// Sonar preference widgets; values are persisted locally until boat-side sync exists.
static lv_obj_t *sonarRange = nullptr;
static lv_obj_t *sonarGain = nullptr;
static lv_obj_t *sonarSensitivity = nullptr;
static lv_obj_t *sonarNoise = nullptr;
static lv_obj_t *sonarPingRate = nullptr;
static lv_obj_t *sonarScrollSpeed = nullptr;
static lv_obj_t *sonarPalette = nullptr;
static lv_obj_t *sonarFishArches = nullptr;
static lv_obj_t *sonarFishAlarm = nullptr;
static lv_obj_t *sonarShallowAlarm = nullptr;
static lv_obj_t *sonarDeepAlarm = nullptr;
static lv_obj_t *sonarShallowDepth = nullptr;
static lv_obj_t *sonarDeepDepth = nullptr;
static lv_obj_t *sonarStatus = nullptr;
static uint16_t screenTimeoutMinutes = 5;

static const lv_color_t BG = lv_color_hex(0x101820);
static const lv_color_t PANEL = lv_color_hex(0x1C2935);
static const lv_color_t TEXT = lv_color_hex(0xFFFFFF);
static const lv_color_t ACCENT = lv_color_hex(0x168AAD);

// Create a consistently styled settings label.
static lv_obj_t *makeLabel(lv_obj_t *parent, const char *value, int x, int y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, value);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, TEXT, LV_PART_MAIN);
    return label;
}

// Create a button with a text label and a click callback.
static lv_obj_t *makeButton(lv_obj_t *parent, const char *value, int x, int y,
                            int width, lv_event_cb_t callback)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, width, 42);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_style_bg_color(button, ACCENT, LV_PART_MAIN);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, value);
    lv_obj_center(label);
    if (callback != nullptr)
        lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);
    return button;
}

// Stop asynchronous Wi-Fi UI polling before its labels are deleted.
static void stopWiFiUiTimers()
{
    if (wifiScanTimer != nullptr)
    {
        lv_timer_del(wifiScanTimer);
        wifiScanTimer = nullptr;
    }
    if (wifiConnectTimer != nullptr)
    {
        lv_timer_del(wifiConnectTimer);
        wifiConnectTimer = nullptr;
    }
}

// Hide the virtual keyboard without deleting the entered text.
static void hideKeyboard()
{
    if (keyboard != nullptr)
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

// Show the LVGL virtual keyboard for the focused Wi-Fi text field.
static void textAreaFocusEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_FOCUSED || keyboard == nullptr)
        return;
    lv_keyboard_set_textarea(keyboard, lv_event_get_target(event));
    lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

// Hide the keyboard when its built-in Ready key is pressed.
static void keyboardEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_READY ||
        lv_event_get_code(event) == LV_EVENT_CANCEL)
        hideKeyboard();
}

// Return to the previous operating screen.
static void backEvent(lv_event_t *event)
{
    (void)event;
    hideKeyboard();
    if (returnToScreen != nullptr)
        lv_screen_load(returnToScreen);
}

// Save non-secret display preferences in NVS.
static void saveDisplayPreferences()
{
    settingsPrefs.begin("flytec", false);
    settingsPrefs.putBool("backlight", lv_obj_has_state(brightnessSwitch, LV_STATE_CHECKED));
    settingsPrefs.putBool("night", lv_obj_has_state(themeSwitch, LV_STATE_CHECKED));
    settingsPrefs.putUShort("timeout", screenTimeoutMinutes);
    settingsPrefs.end();
}

// Persist the backlight switch and apply the board-supported on/off level.
static void brightnessEvent(lv_event_t *event)
{
    (void)event;
    const bool enabled = lv_obj_has_state(brightnessSwitch, LV_STATE_CHECKED);
    displayPort_setBacklight(enabled);
    saveDisplayPreferences();
}

// Save the selected day/night preference. Full palette switching is a later UI pass.
static void themeEvent(lv_event_t *event)
{
    (void)event;
    saveDisplayPreferences();
}

// Cycle the screen timeout through useful values.
static void timeoutEvent(lv_event_t *event)
{
    (void)event;
    if (screenTimeoutMinutes == 1) screenTimeoutMinutes = 3;
    else if (screenTimeoutMinutes == 3) screenTimeoutMinutes = 5;
    else if (screenTimeoutMinutes == 5) screenTimeoutMinutes = 10;
    else screenTimeoutMinutes = 1;
    char value[48];
    snprintf(value, sizeof(value), "Screen timeout: %u min", screenTimeoutMinutes);
    lv_label_set_text(timeoutLabel, value);
    saveDisplayPreferences();
}


// Save the full sonar profile to remote NVS. Applying it to the sonar hardware
// requires a compatible sonar module and a boat-radio settings command, not yet implemented.
static void saveSonarPreferences()
{
    if (sonarRange == nullptr) return;
    settingsPrefs.begin("flytec", false);
    settingsPrefs.putUChar("snr_range", (uint8_t)lv_dropdown_get_selected(sonarRange));
    settingsPrefs.putUChar("snr_gain", (uint8_t)lv_slider_get_value(sonarGain));
    settingsPrefs.putUChar("snr_sens", (uint8_t)lv_slider_get_value(sonarSensitivity));
    settingsPrefs.putUChar("snr_noise", (uint8_t)lv_dropdown_get_selected(sonarNoise));
    settingsPrefs.putUChar("snr_ping", (uint8_t)lv_dropdown_get_selected(sonarPingRate));
    settingsPrefs.putUChar("snr_scroll", (uint8_t)lv_dropdown_get_selected(sonarScrollSpeed));
    settingsPrefs.putUChar("snr_palette", (uint8_t)lv_dropdown_get_selected(sonarPalette));
    settingsPrefs.putBool("snr_arches", lv_obj_has_state(sonarFishArches, LV_STATE_CHECKED));
    settingsPrefs.putBool("snr_falarm", lv_obj_has_state(sonarFishAlarm, LV_STATE_CHECKED));
    settingsPrefs.putBool("snr_salarm", lv_obj_has_state(sonarShallowAlarm, LV_STATE_CHECKED));
    settingsPrefs.putBool("snr_dalarm", lv_obj_has_state(sonarDeepAlarm, LV_STATE_CHECKED));
    settingsPrefs.putUShort("snr_sdepth", (uint16_t)lv_slider_get_value(sonarShallowDepth));
    settingsPrefs.putUShort("snr_ddepth", (uint16_t)lv_slider_get_value(sonarDeepDepth));
    settingsPrefs.end();
    if (sonarStatus) lv_label_set_text(sonarStatus, "Profile saved on remote. Boat/sonar sync pending hardware protocol.");
}

static void sonarPreferenceChanged(lv_event_t *event)
{
    (void)event;
    saveSonarPreferences();
}

static lv_obj_t *makeSonarSlider(lv_obj_t *parent, const char *caption, int y,
                                 int minValue, int maxValue, int initial,
                                 lv_obj_t **store)
{
    makeLabel(parent, caption, 12, y);
    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_pos(slider, 250, y - 2);
    lv_obj_set_size(slider, 230, 18);
    lv_slider_set_range(slider, minValue, maxValue);
    lv_slider_set_value(slider, initial, LV_ANIM_OFF);
    // Persist once when the user releases the slider, not on every drag sample.
    lv_obj_add_event_cb(slider, sonarPreferenceChanged, LV_EVENT_RELEASED, nullptr);
    *store = slider;
    return slider;
}

static lv_obj_t *makeSonarDropdown(lv_obj_t *parent, const char *caption, int y,
                                   const char *options, uint16_t selected,
                                   lv_obj_t **store)
{
    makeLabel(parent, caption, 12, y + 5);
    lv_obj_t *dropdown = lv_dropdown_create(parent);
    lv_obj_set_pos(dropdown, 250, y);
    lv_obj_set_size(dropdown, 230, 34);
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_selected(dropdown, selected);
    lv_obj_add_event_cb(dropdown, sonarPreferenceChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    *store = dropdown;
    return dropdown;
}

static lv_obj_t *makeSonarSwitch(lv_obj_t *parent, const char *caption, int y,
                                 bool enabled, lv_obj_t **store)
{
    makeLabel(parent, caption, 12, y + 3);
    lv_obj_t *sw = lv_switch_create(parent);
    lv_obj_set_pos(sw, 250, y);
    if (enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, sonarPreferenceChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    *store = sw;
    return sw;
}

// Build a broad sonar configuration profile. Exact available controls depend on
// the eventual CHIRP sonar module; these settings are stored but not yet transmitted.
static void buildSonarSection(lv_obj_t *parent)
{
    settingsPrefs.begin("flytec", true);
    const uint8_t range = settingsPrefs.getUChar("snr_range", 2);
    const uint8_t gain = settingsPrefs.getUChar("snr_gain", 55);
    const uint8_t sensitivity = settingsPrefs.getUChar("snr_sens", 60);
    const uint8_t noise = settingsPrefs.getUChar("snr_noise", 1);
    const uint8_t ping = settingsPrefs.getUChar("snr_ping", 1);
    const uint8_t scroll = settingsPrefs.getUChar("snr_scroll", 1);
    const uint8_t palette = settingsPrefs.getUChar("snr_palette", 0);
    const bool arches = settingsPrefs.getBool("snr_arches", true);
    const bool fishAlarm = settingsPrefs.getBool("snr_falarm", false);
    const bool shallowAlarm = settingsPrefs.getBool("snr_salarm", false);
    const bool deepAlarm = settingsPrefs.getBool("snr_dalarm", false);
    const uint16_t shallowDepth = settingsPrefs.getUShort("snr_sdepth", 1);
    const uint16_t deepDepth = settingsPrefs.getUShort("snr_ddepth", 20);
    settingsPrefs.end();

    makeLabel(parent, "SONAR CONFIGURATION", 12, 8);
    makeLabel(parent, "Depth / signal processing", 12, 38);
    makeSonarDropdown(parent, "Maximum depth range", 65,
        "Auto\n5 m\n10 m\n20 m\n30 m\n50 m\n100 m", range, &sonarRange);
    makeSonarSlider(parent, "Gain", 112, 0, 100, gain, &sonarGain);
    makeSonarSlider(parent, "Fish detection sensitivity", 153, 0, 100, sensitivity, &sonarSensitivity);
    makeSonarDropdown(parent, "Noise rejection", 194,
        "Off\nLow\nMedium\nHigh", noise, &sonarNoise);
    makeSonarDropdown(parent, "Ping rate", 235,
        "Low\nNormal\nHigh\nAuto", ping, &sonarPingRate);
    makeSonarDropdown(parent, "Echogram scroll speed", 276,
        "Slow\nNormal\nFast", scroll, &sonarScrollSpeed);
    makeSonarDropdown(parent, "Colour palette", 317,
        "Classic\nHigh contrast\nGrayscale\nNight", palette, &sonarPalette);

    makeLabel(parent, "Display / alarms", 12, 365);
    makeSonarSwitch(parent, "Show fish arches / symbols", 397, arches, &sonarFishArches);
    makeSonarSwitch(parent, "Fish detection alarm", 438, fishAlarm, &sonarFishAlarm);
    makeSonarSwitch(parent, "Shallow-water alarm", 479, shallowAlarm, &sonarShallowAlarm);
    makeSonarSlider(parent, "Shallow alarm depth (m)", 520, 1, 20, shallowDepth, &sonarShallowDepth);
    makeSonarSwitch(parent, "Deep-water alarm", 561, deepAlarm, &sonarDeepAlarm);
    makeSonarSlider(parent, "Deep alarm depth (m)", 602, 1, 100, deepDepth, &sonarDeepDepth);
    sonarStatus = makeLabel(parent, "Changes save automatically on the remote.", 12, 650);
}

// Poll an asynchronous Wi-Fi scan without blocking LVGL touch and display updates.
static void wifiScanPoll(lv_timer_t *timer)
{
    const int result = WiFi.scanComplete();
    if (result == WIFI_SCAN_RUNNING)
        return;

    if (wifiScanTimer != nullptr)
    {
        lv_timer_del(wifiScanTimer);
        wifiScanTimer = nullptr;
    }

    if (result == WIFI_SCAN_FAILED)
    {
        lv_label_set_text(wifiStatus, "Wi-Fi scan failed. Enter SSID manually.");
        WiFi.scanDelete();
        return;
    }

    String output;
    const int shown = (result > 8) ? 8 : result;
    for (int i = 0; i < shown; ++i)
    {
        output += String(i + 1) + ". " + WiFi.SSID(i) + " (" +
                  String(WiFi.RSSI(i)) + " dBm)\n";
    }
    if (result == 0)
        output = "No networks found. Enter SSID manually.";
    else
        output += "Select or type the network name in the SSID field.";

    lv_label_set_text(scanResults, output.c_str());
    lv_label_set_text(wifiStatus, "Scan complete.");
    WiFi.scanDelete();
    (void)timer;
}

// Start a non-blocking Wi-Fi scan.
static void scanWiFiEvent(lv_event_t *event)
{
    (void)event;
    if (wifiScanTimer != nullptr)
        return;

    lv_label_set_text(wifiStatus, "Scanning Wi-Fi networks...");
    WiFi.mode(WIFI_STA);
    const int result = WiFi.scanNetworks(true, true); // Async scan; show hidden networks as well.
    if (result == WIFI_SCAN_FAILED)
    {
        lv_label_set_text(wifiStatus, "Could not start Wi-Fi scan.");
        return;
    }
    wifiScanTimer = lv_timer_create(wifiScanPoll, 250, nullptr);
}

// Poll Wi-Fi association without blocking the LVGL event loop.
static void wifiConnectPoll(lv_timer_t *timer)
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        lv_label_set_text(wifiStatus, "Connected to Wi-Fi.");
        lv_timer_del(timer);
        wifiConnectTimer = nullptr;
        return;
    }
    if (static_cast<uint32_t>(millis() - wifiConnectStartedAt) >= 20000U)
    {
        lv_label_set_text(wifiStatus, "Connection timed out. Check SSID/password.");
        lv_timer_del(timer);
        wifiConnectTimer = nullptr;
    }
}

// Save the Wi-Fi profile locally, then start a non-blocking connection attempt.
static void connectWiFiEvent(lv_event_t *event)
{
    (void)event;
    const char *ssid = lv_textarea_get_text(ssidArea);
    const char *password = lv_textarea_get_text(passwordArea);
    if (ssid == nullptr || ssid[0] == '\0')
    {
        lv_label_set_text(wifiStatus, "Enter a Wi-Fi network name first.");
        return;
    }

    if (wifiConnectTimer != nullptr)
    {
        lv_timer_del(wifiConnectTimer);
        wifiConnectTimer = nullptr;
    }

    settingsPrefs.begin("flytec", false);
    settingsPrefs.putString("wifi_ssid", ssid);
    settingsPrefs.putString("wifi_pass", password != nullptr ? password : "");
    settingsPrefs.end();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password != nullptr ? password : "");
    wifiConnectStartedAt = millis();
    lv_label_set_text(wifiStatus, "Connecting to Wi-Fi...");
    wifiConnectTimer = lv_timer_create(wifiConnectPoll, 500, nullptr);
}

// Build the Wi-Fi category, including text fields and the virtual keyboard.
static void buildWiFiSection(lv_obj_t *parent)
{
    makeLabel(parent, "WI-FI NETWORK", 18, 12);
    makeLabel(parent, "Network name (SSID)", 18, 52);
    ssidArea = lv_textarea_create(parent);
    lv_obj_set_pos(ssidArea, 18, 78);
    lv_obj_set_size(ssidArea, 330, 44);
    lv_textarea_set_one_line(ssidArea, true);
    lv_textarea_set_placeholder_text(ssidArea, "Enter Wi-Fi name");
    lv_obj_add_event_cb(ssidArea, textAreaFocusEvent, LV_EVENT_FOCUSED, nullptr);

    makeLabel(parent, "Password", 18, 134);
    passwordArea = lv_textarea_create(parent);
    lv_obj_set_pos(passwordArea, 18, 160);
    lv_obj_set_size(passwordArea, 330, 44);
    lv_textarea_set_one_line(passwordArea, true);
    lv_textarea_set_password_mode(passwordArea, true);
    lv_textarea_set_placeholder_text(passwordArea, "Enter Wi-Fi password");
    lv_obj_add_event_cb(passwordArea, textAreaFocusEvent, LV_EVENT_FOCUSED, nullptr);

    makeButton(parent, "SCAN", 370, 78, 120, scanWiFiEvent);
    makeButton(parent, "CONNECT", 370, 160, 120, connectWiFiEvent);
    wifiStatus = makeLabel(parent, "Wi-Fi is not connected.", 18, 220);
    scanResults = makeLabel(parent, "Nearby networks will appear here.", 18, 250);

    if (keyboard == nullptr)
    {
        keyboard = lv_keyboard_create(settingsScreen);
        lv_obj_set_size(keyboard, 800, 205);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_obj_add_event_cb(keyboard, keyboardEvent, LV_EVENT_READY, nullptr);
        lv_obj_add_event_cb(keyboard, keyboardEvent, LV_EVENT_CANCEL, nullptr);
    }
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

// Build the display category with controls supported by the present board configuration.
static void buildDisplaySection(lv_obj_t *parent)
{
    makeLabel(parent, "DISPLAY", 18, 12);
    makeLabel(parent, "Backlight power", 18, 62);
    brightnessSwitch = lv_switch_create(parent);
    lv_obj_set_pos(brightnessSwitch, 250, 52);
    lv_obj_add_event_cb(brightnessSwitch, brightnessEvent, LV_EVENT_VALUE_CHANGED, nullptr);
    makeLabel(parent, "The current CH422G backlight setup supports ON/OFF only.", 18, 105);
    makeLabel(parent, "Variable brightness requires a PWM-capable backlight configuration.", 18, 132);

    makeLabel(parent, "Night theme preference (saved; theme switching pending)", 18, 185);
    themeSwitch = lv_switch_create(parent);
    lv_obj_set_pos(themeSwitch, 250, 175);
    lv_obj_add_event_cb(themeSwitch, themeEvent, LV_EVENT_VALUE_CHANGED, nullptr);
    timeoutLabel = makeLabel(parent, "Screen timeout: 5 min (setting saved; dimming not yet active)", 18, 245);
    makeButton(parent, "CHANGE TIMEOUT", 18, 280, 190, timeoutEvent);
}

// Build a clear placeholder for categories whose hardware integration is not yet complete.
static void buildInfoSection(lv_obj_t *parent, const char *heading, const char *description)
{
    makeLabel(parent, heading, 18, 12);
    makeLabel(parent, description, 18, 65);
}


// ------------------ CONTROLS SETTINGS ------------------
static lv_obj_t *controlDeadZoneSlider = nullptr;
static lv_obj_t *controlDeadZoneLabel = nullptr;
static lv_obj_t *controlLayoutLabel = nullptr;
static lv_obj_t *controlOverrideLabel = nullptr;

static void layoutSettingsEvent(lv_event_t *event)
{
    (void)event;
    const JoystickSnapshot snap=joystick_getSnapshot();
    if(!snap.readingsValid || !snap.neutral){
        lv_label_set_text(controlLayoutLabel,"Center sticks first; layout unchanged.");
        return;
    }
    joystick_setLayout(joystick_getLayout()==CONTROL_TANK?CONTROL_STEERING:CONTROL_TANK);
    lv_label_set_text(controlLayoutLabel,joystick_getLayout()==CONTROL_TANK?"Layout: TANK":"Layout: STEERING");
}
static void overrideSettingsEvent(lv_event_t *event)
{
    (void)event;
    joystick_setAutopilotOverride(joystick_getAutopilotOverride()==OVERRIDE_TEMPORARY?OVERRIDE_TO_MANUAL:OVERRIDE_TEMPORARY);
    lv_label_set_text(controlOverrideLabel,joystick_getAutopilotOverride()==OVERRIDE_TEMPORARY?"Override: TEMPORARY CONTROL":"Override: SWITCH TO MANUAL");
}
static void controlDeadZoneEvent(lv_event_t *event)
{
    (void)event;
    const int value=lv_slider_get_value(controlDeadZoneSlider);
    joystick_setDeadZone(value);
    char label[64]; snprintf(label,sizeof(label),"Dead zone: %d ADC counts",value);
    lv_label_set_text(controlDeadZoneLabel,label);
}
static void openCalibrationEvent(lv_event_t *event)
{
    (void)event;
    joystickCalibrationUI_open(returnToScreen);
}
static void buildControlsSection(lv_obj_t *parent)
{
    makeLabel(parent,"JOYSTICK / CONTROL",12,12);
    controlLayoutLabel=makeLabel(parent,joystick_getLayout()==CONTROL_TANK?"Layout: TANK":"Layout: STEERING",12,54);
    makeButton(parent,"TOGGLE LAYOUT",340,48,180,layoutSettingsEvent);
    controlOverrideLabel=makeLabel(parent,joystick_getAutopilotOverride()==OVERRIDE_TEMPORARY?"Override: TEMPORARY CONTROL":"Override: SWITCH TO MANUAL",12,105);
    makeButton(parent,"TOGGLE OVERRIDE",340,98,180,overrideSettingsEvent);
    JoystickSnapshot snap=joystick_getSnapshot();
    makeLabel(parent,"Dead zone",12,158);
    controlDeadZoneSlider=lv_slider_create(parent);
    lv_obj_set_pos(controlDeadZoneSlider,220,156);
    lv_obj_set_size(controlDeadZoneSlider,300,24);
    lv_slider_set_range(controlDeadZoneSlider,10,120);
    lv_slider_set_value(controlDeadZoneSlider,snap.calibration.deadZone,LV_ANIM_OFF);
    lv_obj_add_event_cb(controlDeadZoneSlider,controlDeadZoneEvent,LV_EVENT_RELEASED,nullptr);
    char label[64];snprintf(label,sizeof(label),"Dead zone: %d ADC counts",snap.calibration.deadZone);
    controlDeadZoneLabel=makeLabel(parent,label,12,194);
    makeLabel(parent,"Calibration requires a confirmed boat STOP; motor output is inhibited while calibrating.",12,240);
    makeButton(parent,"OPEN CALIBRATION",12,282,220,openCalibrationEvent);
}

// Switch the right-hand settings panel when a category is selected.
static void categoryEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED || settingsContent == nullptr)
        return;
    const char *category = static_cast<const char *>(lv_event_get_user_data(event));
    hideKeyboard();
    stopWiFiUiTimers();
    lv_obj_clean(settingsContent);
    ssidArea = nullptr;
    passwordArea = nullptr;
    sonarRange = sonarGain = sonarSensitivity = sonarNoise = nullptr;
    sonarPingRate = sonarScrollSpeed = sonarPalette = nullptr;
    sonarFishArches = sonarFishAlarm = sonarShallowAlarm = sonarDeepAlarm = nullptr;
    sonarShallowDepth = sonarDeepDepth = sonarStatus = nullptr;
    if (strcmp(category, "WIFI") == 0)
    {
        buildWiFiSection(settingsContent);
        settingsPrefs.begin("flytec", true);
        const String savedSsid = settingsPrefs.getString("wifi_ssid", "");
        const String savedPass = settingsPrefs.getString("wifi_pass", "");
        settingsPrefs.end();
        lv_textarea_set_text(ssidArea, savedSsid.c_str());
        lv_textarea_set_text(passwordArea, savedPass.c_str());
    }
    else if (strcmp(category, "DISPLAY") == 0)
    {
        buildDisplaySection(settingsContent);
        settingsPrefs.begin("flytec", true);
        const bool backlightOn = settingsPrefs.getBool("backlight", true);
        const bool nightMode = settingsPrefs.getBool("night", false);
        screenTimeoutMinutes = settingsPrefs.getUShort("timeout", 5);
        settingsPrefs.end();
        if (backlightOn) lv_obj_add_state(brightnessSwitch, LV_STATE_CHECKED);
        else lv_obj_clear_state(brightnessSwitch, LV_STATE_CHECKED);
        if (nightMode) lv_obj_add_state(themeSwitch, LV_STATE_CHECKED);
        else lv_obj_clear_state(themeSwitch, LV_STATE_CHECKED);
        char timeoutText[96];
        snprintf(timeoutText, sizeof(timeoutText),
                 "Screen timeout: %u min (preference saved; automatic dimming not active)",
                 static_cast<unsigned>(screenTimeoutMinutes));
        lv_label_set_text(timeoutLabel, timeoutText);
        displayPort_setBacklight(backlightOn);
    }
    else if (strcmp(category, "CONTROLS") == 0)
        buildControlsSection(settingsContent);
    else if (strcmp(category, "SONAR") == 0)
        buildSonarSection(settingsContent);
    else if (strcmp(category, "MAP") == 0)
        buildInfoSection(settingsContent, "MAP PREFERENCES", "Map style, zoom and boat-up settings will be connected to the offline map renderer.");
    else
        buildInfoSection(settingsContent, "SYSTEM", "Firmware version, storage status and restart controls will be added after hardware validation.");
}

// Create a navigation button and associate its settings category.
static void makeCategoryButton(lv_obj_t *parent, const char *title, const char *category, int y)
{
    lv_obj_t *button = makeButton(parent, title, 8, y, 145, nullptr);
    lv_obj_add_event_cb(button, categoryEvent, LV_EVENT_CLICKED, const_cast<char *>(category));
}

// Load preferences and create the complete settings window.
void settingsUI_open(lv_obj_t *returnScreen)
{
    returnToScreen = returnScreen;
    // Reuse the settings screen: this also keeps the virtual keyboard parent valid
    // and prevents allocating another full screen every time Settings is opened.
    if (settingsScreen != nullptr)
    {
        lv_screen_load(settingsScreen);
        return;
    }
    settingsScreen = lv_obj_create(nullptr);
    lv_obj_set_size(settingsScreen, 800, 480);
    lv_obj_set_style_bg_color(settingsScreen, BG, LV_PART_MAIN);
    lv_obj_set_style_pad_all(settingsScreen, 0, LV_PART_MAIN);
    lv_obj_clear_flag(settingsScreen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *header = lv_obj_create(settingsScreen);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 800, 55);
    lv_obj_set_style_bg_color(header, PANEL, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);
    makeLabel(header, "REMOTE SETTINGS", 18, 17);
    makeButton(header, "BACK", 690, 7, 95, backEvent);

    lv_obj_t *nav = lv_obj_create(settingsScreen);
    lv_obj_set_pos(nav, 8, 65);
    lv_obj_set_size(nav, 170, 405);
    lv_obj_set_style_bg_color(nav, PANEL, LV_PART_MAIN);
    lv_obj_set_style_pad_all(nav, 8, LV_PART_MAIN);

    settingsContent = lv_obj_create(settingsScreen);
    lv_obj_set_pos(settingsContent, 188, 65);
    lv_obj_set_size(settingsContent, 604, 405);
    lv_obj_set_style_bg_color(settingsContent, PANEL, LV_PART_MAIN);
    lv_obj_set_style_pad_all(settingsContent, 8, LV_PART_MAIN);
    lv_obj_add_flag(settingsContent, LV_OBJ_FLAG_SCROLLABLE);

    // This first settings release opens the Wi-Fi category by default.
    makeLabel(nav, "CATEGORIES", 8, 8);
    makeCategoryButton(nav, "WI-FI", "WIFI", 48);
    makeCategoryButton(nav, "DISPLAY", "DISPLAY", 98);
    makeCategoryButton(nav, "CONTROLS", "CONTROLS", 148);
    makeCategoryButton(nav, "SONAR", "SONAR", 198);
    makeCategoryButton(nav, "MAP", "MAP", 248);
    makeCategoryButton(nav, "SYSTEM", "SYSTEM", 298);

    settingsPrefs.begin("flytec", true);
    const bool backlightOn = settingsPrefs.getBool("backlight", true);
    screenTimeoutMinutes = settingsPrefs.getUShort("timeout", 5);
    const String savedSsid = settingsPrefs.getString("wifi_ssid", "");
    const String savedPass = settingsPrefs.getString("wifi_pass", "");
    settingsPrefs.end();

    buildWiFiSection(settingsContent);
    lv_textarea_set_text(ssidArea, savedSsid.c_str());
    lv_textarea_set_text(passwordArea, savedPass.c_str());

    // Apply the persisted hardware setting without touching Display widgets
    // that have not been created yet. The Display category restores its own controls.
    displayPort_setBacklight(backlightOn);

    // The Wi-Fi category is the initial view; category buttons switch the content panel.
    lv_screen_load(settingsScreen);
}
