
#include "joystick_calibration_ui.h"

#include "joystick.h"
#include "boat_state.h"

// --------------------------------------------------
// SCREEN OBJECTS
// --------------------------------------------------

static lv_obj_t *calScreen = nullptr;
static lv_obj_t *homeScreen = nullptr;

static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *layoutLabel = nullptr;
static lv_obj_t *overrideLabel = nullptr;
static lv_obj_t *deadZoneLabel = nullptr;

static lv_obj_t *axisLabels[4] = {};

static lv_obj_t *enterButton = nullptr;
static lv_obj_t *centreButton = nullptr;
static lv_obj_t *endpointButton = nullptr;
static lv_obj_t *finishButton = nullptr;

static lv_obj_t *layoutButton = nullptr;
static lv_obj_t *overrideButton = nullptr;
static lv_obj_t *deadZoneSlider = nullptr;

// Refresh timer, executed by LVGL.
static lv_timer_t *refreshTimer = nullptr;

// --------------------------------------------------
// COLOURS
// --------------------------------------------------

static const lv_color_t BG =
    lv_color_hex(0x081522);

static const lv_color_t PANEL =
    lv_color_hex(0x10243A);

static const lv_color_t TEXT =
    lv_color_hex(0xFFFFFF);

static const lv_color_t GREEN =
    lv_color_hex(0x20C979);

static const lv_color_t RED =
    lv_color_hex(0xD32F2F);

static const lv_color_t BLUE =
    lv_color_hex(0x174F92);

// --------------------------------------------------
// CREATE LABEL
// --------------------------------------------------

static lv_obj_t *makeLabel(
    lv_obj_t *parent,
    const char *text,
    int x,
    int y,
    int width)
{
    // Create a label.
    lv_obj_t *label = lv_label_create(parent);

    // Set its initial text.
    lv_label_set_text(label, text);

    // Set its position.
    lv_obj_set_pos(label, x, y);

    // Set its width.
    lv_obj_set_width(label, width);

    // Set its text colour.
    lv_obj_set_style_text_color(
        label,
        TEXT,
        LV_PART_MAIN
    );

    // Return the label.
    return label;
}

// --------------------------------------------------
// CREATE BUTTON
// --------------------------------------------------

static lv_obj_t *makeButton(
    lv_obj_t *parent,
    const char *text,
    int x,
    int y,
    int width,
    int height,
    lv_color_t colour)
{
    // Create an LVGL 9 button.
    lv_obj_t *button = lv_button_create(parent);

    // Set button dimensions.
    lv_obj_set_size(button, width, height);

    // Position the button.
    lv_obj_set_pos(button, x, y);

    // Set button background colour.
    lv_obj_set_style_bg_color(button, colour, LV_PART_MAIN);
    lv_obj_set_style_radius(button, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(0x347FC0), LV_PART_MAIN);
    lv_obj_set_style_text_color(button, TEXT, LV_PART_MAIN);

    // Create button text.
    lv_obj_t *label = lv_label_create(button);

    // Set button text.
    lv_label_set_text(label, text);

    // Centre the text.
    lv_obj_center(label);

    // Return the button.
    return button;
}

// --------------------------------------------------
// SET STATUS MESSAGE
// --------------------------------------------------

static void setStatus(const char *message)
{
    // Update the status message.
    lv_label_set_text(statusLabel, message);
}

// --------------------------------------------------
// ENTER CALIBRATION
// --------------------------------------------------

static void enterCalibrationEvent(lv_event_t *event)
{
    // Ignore events other than button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Attempt to enter protected calibration mode.
    if (joystick_calibrationEnter())
    {
        setStatus("КАЛИБРАЦИЯ — МОТОРИТЕ СА ИЗКЛЮЧЕНИ");
    }
    else
    {
        setStatus("БЛОКИРАНО: ПОТВЪРДИ STOP НА ЛОДКАТА");
    }
}

// --------------------------------------------------
// CAPTURE CENTRE
// --------------------------------------------------

static void centreCalibrationEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Capture the centre of all four axes.
    if (joystick_calibrateCentre())
    {
        setStatus("ЗАПИС НА ЦЕНТЪР — 0/32");
    }
    else
    {
        setStatus("ГРЕШКА — ПРОВЕРИ ADC");
    }
}

// --------------------------------------------------
// START ENDPOINT CAPTURE
// --------------------------------------------------

static void startEndpointEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Start recording axis endpoints.
    if (joystick_beginEndpointCapture())
    {
        setStatus("ДВИЖИ 4-ТЕ СТИКА ДО КРАЙНИТЕ ПОЗИЦИИ");
    }
    else
    {
        setStatus("НЕ МОЖЕ ДА СТАРТИРА ЗАПИС НА КРАЙНИТЕ");
    }
}

// --------------------------------------------------
// FINISH ENDPOINT CAPTURE
// --------------------------------------------------

static void finishEndpointEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Validate and save the captured endpoints in RAM.
    if (joystick_endEndpointCapture())
    {
        setStatus("КРАЙНИТЕ СА ПРИЕТИ — НЕ СА ЗАПИСАНИ ВЪВ FLASH");
    }
    else
    {
        setStatus("НЕВАЛИДНИ КРАЙНИ — ДВИЖИ СТИКОВЕТЕ ДОКРАЙ");
    }
}

// --------------------------------------------------
// CHANGE CONTROL LAYOUT
// --------------------------------------------------

static void layoutEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Get current joystick readings.
    JoystickSnapshot data = joystick_getSnapshot();

    // Require valid readings and neutral sticks.
    if (!data.readingsValid || !data.neutral)
    {
        setStatus("ЦЕНТРИРАЙ СТИКОВЕТЕ ПРЕДИ СМЯНА НА РЕЖИМА");
        return;
    }

    // Switch to the other layout.
    if (joystick_getLayout() == CONTROL_TANK)
    {
        joystick_setLayout(CONTROL_STEERING);
    }
    else
    {
        joystick_setLayout(CONTROL_TANK);
    }

    // Refresh the settings label.
    if (joystick_getLayout() == CONTROL_TANK)
    {
        lv_label_set_text(layoutLabel, "УПРАВЛЕНИЕ: TANK");
    }
    else
    {
        lv_label_set_text(layoutLabel, "УПРАВЛЕНИЕ: STEERING");
    }

    setStatus("РЕЖИМЪТ НА УПРАВЛЕНИЕ Е СМЕНЕН");
}

// --------------------------------------------------
// CHANGE AUTOPILOT OVERRIDE
// --------------------------------------------------

static void overrideEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    // Switch between the two override behaviours.
    if (joystick_getAutopilotOverride() == OVERRIDE_TEMPORARY)
    {
        joystick_setAutopilotOverride(OVERRIDE_TO_MANUAL);

        lv_label_set_text(
            overrideLabel,
            "АВТОПИЛОТ: ПРЕВКЛЮЧВА КЪМ РЪЧНО"
        );
    }
    else
    {
        joystick_setAutopilotOverride(OVERRIDE_TEMPORARY);

        lv_label_set_text(
            overrideLabel,
            "АВТОПИЛОТ: ВРЕМЕНЕН OVERRIDE"
        );
    }

    setStatus("OVERRIDE НАСТРОЙКАТА Е ОБНОВЕНА");
}

// --------------------------------------------------
// DEAD ZONE SLIDER
// --------------------------------------------------

static void deadZoneEvent(lv_event_t *event)
{
    // Only handle slider value changes.
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED)
    {
        return;
    }

    // Get the slider object.
    lv_obj_t *slider = lv_event_get_target_obj(event);

    // Read the selected dead-zone value.
    int value = lv_slider_get_value(slider);

    // Update joystick calibration.
    joystick_setDeadZone(value);

    // Display the selected value.
    char text[40];

    snprintf(
        text,
        sizeof(text),
        "МЪРТВА ЗОНА: %d ADC",
        value
    );

    lv_label_set_text(deadZoneLabel, text);
}

// --------------------------------------------------
// RETURN TO MAIN SCREEN
// --------------------------------------------------

static void closeEvent(lv_event_t *event)
{
    // Only handle button clicks.
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    joystickCalibrationUI_close();
}

// --------------------------------------------------
// UPDATE AXIS DISPLAY
// --------------------------------------------------

static void updateAxisLabel(
    lv_obj_t *label,
    const char *name,
    int raw,
    const AxisCalibration &axis,
    int normalised)
{
    // Prepare a formatted axis information line.
    char text[160];

    snprintf(
        text,
        sizeof(text),
        "%s  RAW:%4d  MIN:%4d  CTR:%4d  MAX:%4d  OUT:%+5d",
        name,
        raw,
        axis.minimum,
        axis.centre,
        axis.maximum,
        normalised
    );

    // Update the label.
    lv_label_set_text(label, text);
}

// --------------------------------------------------
// PERIODIC SCREEN REFRESH
// --------------------------------------------------

static void refreshScreen(lv_timer_t *timer)
{
    // Get the newest joystick data.
    JoystickSnapshot data = joystick_getSnapshot();

    // Display reading validity.
    if (!data.readingsValid)
    {
        setStatus("ГРЕШКА ПРИ ЧЕТЕНЕ — МОТОРИТЕ СА БЛОКИРАНИ");
    }
    else if (data.calibrationMode && data.endpointCapture)
    {
        setStatus("ЗАПИС НА КРАЙНИ — МОТОРИТЕ СА ИЗКЛЮЧЕНИ");
    }
    else if (data.calibrationMode && data.centreCaptureState == CENTRE_RUNNING)
    {
        char progress[96];
        snprintf(progress,sizeof(progress),"ЗАПИС НА ЦЕНТЪР — %u/32",static_cast<unsigned>(data.centreSamples));
        setStatus(progress);
    }
    else if (data.calibrationMode && data.centreCaptureState == CENTRE_FAILED)
    {
        setStatus("ГРЕШКА ПРИ ЗАПИС НА ЦЕНТЪР — ПОВТОРИ");
    }
    else if (data.calibrationMode && data.centreCaptureState == CENTRE_COMPLETE && !data.calibrationValid)
    {
        setStatus("ЦЕНТЪРЪТ Е ЗАПИСАН — ЗАПИШИ КРАЙНИ");
    }
    else if (data.calibrationMode)
    {
        setStatus("КАЛИБРАЦИЯ — МОТОРИТЕ СА БЛОКИРАНИ");
    }
    else if (!boatState.emergencyStopConfirmed ||
             !boatState.emergencyStop)
    {
        setStatus("КАЛИБРАЦИЯТА Е БЛОКИРАНА — ПОТВЪРДИ STOP");
    }
    else if (!data.calibrationValid)
    {
        setStatus("НЕСЪПРОВЕРЕНИ — ЗАПИШИ ЦЕНТЪР И КРАЙНИ");
    }
    else if (data.neutral)
    {
        setStatus("СТИКОВЕТЕ СА В НЕУТРАЛНА ПОЗИЦИЯ");
    }
    else
    {
        setStatus("ОТЧЕТЕНО ДВИЖЕНИЕ НА СТИК");
    }

    // Update the left X axis line.
    updateAxisLabel(
        axisLabels[0],
        "ЛЯВ X ",
        data.raw.leftX,
        data.calibration.leftX,
        data.values.leftX
    );

    // Update the left Y axis line.
    updateAxisLabel(
        axisLabels[1],
        "ЛЯВ Y ",
        data.raw.leftY,
        data.calibration.leftY,
        data.values.leftY
    );

    // Update the right X axis line.
    updateAxisLabel(
        axisLabels[2],
        "ДЕСЕН X",
        data.raw.rightX,
        data.calibration.rightX,
        data.values.rightX
    );

    // Update the right Y axis line.
    updateAxisLabel(
        axisLabels[3],
        "ДЕСЕН Y",
        data.raw.rightY,
        data.calibration.rightY,
        data.values.rightY
    );

    // Refresh the layout label.
    if (joystick_getLayout() == CONTROL_TANK)
    {
        lv_label_set_text(layoutLabel, "УПРАВЛЕНИЕ: TANK");
    }
    else
    {
        lv_label_set_text(layoutLabel, "УПРАВЛЕНИЕ: STEERING");
    }

    // Refresh the dead-zone label.
    char deadText[40];

    snprintf(
        deadText,
        sizeof(deadText),
        "МЪРТВА ЗОНА: %d ADC",
        data.calibration.deadZone
    );

    lv_label_set_text(deadZoneLabel, deadText);

    // Refresh the override label.
    if (joystick_getAutopilotOverride() == OVERRIDE_TEMPORARY)
    {
        lv_label_set_text(
            overrideLabel,
            "АВТОПИЛОТ: ВРЕМЕНЕН"
        );
    }
    else
    {
        lv_label_set_text(
            overrideLabel,
            "AUTOPILOT OVERRIDE: SWITCH TO MANUAL"
        );
    }

    // Keep the timer parameter explicitly unused.
    (void)timer;
}

// --------------------------------------------------
// CREATE CALIBRATION SCREEN
// --------------------------------------------------

void joystickCalibrationUI_open(lv_obj_t *returnScreen)
{
    // Remember the screen to return to.
    homeScreen = returnScreen;

    // Create a new LVGL screen.
    calScreen = lv_obj_create(NULL);

    // Set the background colour.
    lv_obj_set_style_bg_color(
        calScreen,
        BG,
        LV_PART_MAIN
    );

    // Disable scrolling.
    lv_obj_remove_flag(
        calScreen,
        LV_OBJ_FLAG_SCROLLABLE
    );

    // --------------------------------------------------
    // TITLE
    // --------------------------------------------------

    makeLabel(
        calScreen,
        "КАЛИБРАЦИЯ НА СТИКОВЕ / УПРАВЛЕНИЕ",
        15,
        10,
        760
    );

    // --------------------------------------------------
    // STATUS
    // --------------------------------------------------

    statusLabel = makeLabel(
        calScreen,
        "ОЧАКВАНЕ НА ДАННИ ОТ СТИКОВЕТЕ",
        15,
        43,
        770
    );

    // --------------------------------------------------
    // AXIS INFORMATION PANEL
    // --------------------------------------------------

    lv_obj_t *axisPanel = lv_obj_create(calScreen);

    lv_obj_set_pos(axisPanel, 10, 75);
    lv_obj_set_size(axisPanel, 780, 185);

    lv_obj_set_style_bg_color(
        axisPanel,
        PANEL,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        axisPanel,
        10,
        LV_PART_MAIN
    );

    // Axis column headings.
    makeLabel(
        axisPanel,
        "ОС     RAW     МИН     ЦЕНТЪР     МАКС     НОРМ.",
        5,
        5,
        750
    );

    // Left horizontal axis.
    axisLabels[0] = makeLabel(
        axisPanel,
        "ЛЯВ X",
        5,
        40,
        750
    );

    // Left vertical axis.
    axisLabels[1] = makeLabel(
        axisPanel,
        "ЛЯВ Y",
        5,
        75,
        750
    );

    // Right horizontal axis.
    axisLabels[2] = makeLabel(
        axisPanel,
        "ДЕСЕН X",
        5,
        110,
        750
    );

    // Right vertical axis.
    axisLabels[3] = makeLabel(
        axisPanel,
        "ДЕСЕН Y",
        5,
        145,
        750
    );

    // --------------------------------------------------
    // CALIBRATION CONTROLS
    // --------------------------------------------------

    enterButton = makeButton(
        calScreen,
        "ВХОД КАЛИБР.",
        15,
        275,
        185,
        42,
        BLUE
    );

    lv_obj_add_event_cb(
        enterButton,
        enterCalibrationEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    centreButton = makeButton(
        calScreen,
        "ЗАПИС ЦЕНТЪР",
        210,
        275,
        175,
        42,
        GREEN
    );

    lv_obj_add_event_cb(
        centreButton,
        centreCalibrationEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    endpointButton = makeButton(
        calScreen,
        "ЗАПИС КРАЙНИ",
        395,
        275,
        175,
        42,
        BLUE
    );

    lv_obj_add_event_cb(
        endpointButton,
        startEndpointEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    finishButton = makeButton(
        calScreen,
        "ЗАВЪРШИ",
        580,
        275,
        200,
        42,
        GREEN
    );

    lv_obj_add_event_cb(
        finishButton,
        finishEndpointEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    // --------------------------------------------------
    // CONTROL SETTINGS
    // --------------------------------------------------

    layoutButton = makeButton(
        calScreen,
        "СМЯНА РЕЖИМ",
        15,
        330,
        180,
        42,
        BLUE
    );

    layoutLabel = makeLabel(
        calScreen,
        "УПРАВЛЕНИЕ: TANK",
        205,
        342,
        180
    );

    lv_obj_add_event_cb(
        layoutButton,
        layoutEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    overrideButton = makeButton(
        calScreen,
        "СМЯНА OVERRIDE",
        395,
        330,
        180,
        42,
        BLUE
    );

    overrideLabel = makeLabel(
        calScreen,
        "AUTOPILOT OVERRIDE: TEMPORARY",
        15,
        385,
        760
    );

    lv_obj_add_event_cb(
        overrideButton,
        overrideEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    // --------------------------------------------------
    // DEAD ZONE SETTING
    // --------------------------------------------------

    deadZoneLabel = makeLabel(
        calScreen,
        "МЪРТВА ЗОНА: 65 ADC",
        15,
        415,
        260
    );

    deadZoneSlider = lv_slider_create(calScreen);

    lv_obj_set_pos(deadZoneSlider, 285, 415);
    lv_obj_set_size(deadZoneSlider, 280, 15);

    // Set the allowed dead-zone range.
    lv_slider_set_range(deadZoneSlider, 10, 120);

    // Set the current dead-zone value.
    lv_slider_set_value(
        deadZoneSlider,
        joystick_getSnapshot().calibration.deadZone,
        LV_ANIM_OFF
    );

    // Register the slider callback.
    lv_obj_add_event_cb(
        deadZoneSlider,
        deadZoneEvent,
        LV_EVENT_VALUE_CHANGED,
        nullptr
    );

    // --------------------------------------------------
    // RETURN BUTTON
    // --------------------------------------------------

    lv_obj_t *backButton = makeButton(
        calScreen,
        "НАЗАД",
        650,
        400,
        130,
        45,
        RED
    );

    lv_obj_add_event_cb(
        backButton,
        closeEvent,
        LV_EVENT_CLICKED,
        nullptr
    );

    // --------------------------------------------------
    // SHOW SCREEN
    // --------------------------------------------------

    lv_screen_load(calScreen);

    // Refresh the display every 100 milliseconds.
    refreshTimer = lv_timer_create(
        refreshScreen,
        100,
        nullptr
    );
}

// --------------------------------------------------
// CLOSE CALIBRATION SCREEN
// --------------------------------------------------

void joystickCalibrationUI_close()
{
    if(!calScreen||lv_screen_active()!=calScreen)return;
    // Require all four axes to be neutral before leaving.
    if (!joystick_calibrationExit())
    {
        setStatus("ЦЕНТРИРАЙ СТИКОВЕТЕ ЗА ИЗХОД");
        return;
    }

    // Stop the screen refresh timer.
    if (refreshTimer != nullptr)
    {
        lv_timer_delete(refreshTimer);
        refreshTimer = nullptr;
    }

    // Return to the original main screen.
    if (homeScreen != nullptr)
    {
        lv_screen_load(homeScreen);
    }

    // Retire screen and timer together, after LVGL has switched to parent.
    lv_obj_t *old=calScreen;calScreen=nullptr;lv_obj_delete_async(old);
}

bool joystickCalibrationUI_isOpen(){return calScreen&&lv_screen_active()==calScreen;}
