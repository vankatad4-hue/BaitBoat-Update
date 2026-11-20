// Waveshare ESP32-S3-Touch-LCD-4.3 (standard, SKU 25948) display adapter.
// Hardware initialisation follows the uploaded official Waveshare Arduino example.
// LVGL registration below targets LVGL 9.6.x APIs (not the demo's LVGL 8 port).

#include "display_port.h"                         // Public adapter interface.
#include <Arduino.h>                               // Arduino runtime and millis().
#include <lvgl.h>                                  // LVGL 9.6 public API.
#include <esp_display_panel.hpp>                  // Waveshare/Espressif board, LCD and touch classes.
#include <esp_heap_caps.h>                         // PSRAM-capable memory allocation.
#include <FS.h>                                    // Arduino filesystem types.
#include <SD.h>                                    // Arduino SD-card filesystem.
#include <SPI.h>                                   // Hardware SPI bus for the TF card.

using namespace esp_panel::drivers;               // Expose LCD and Touch driver classes.
using namespace esp_panel::board;                 // Expose the supported Board class.

// Stop compilation if an incompatible LVGL major/minor release is selected.
#if !defined(LVGL_VERSION_MAJOR) || !defined(LVGL_VERSION_MINOR)
#error "LVGL version macros are unavailable; install LVGL 9.6.x."
#endif
#if LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR != 6
#error "This Flytec remote source targets LVGL 9.6.x. Select LVGL 9.6.0 or a compatible 9.6 patch release."
#endif

static Board *g_board = nullptr;                  // Own the Waveshare board object for the life of the app.
static LCD *g_lcd = nullptr;                      // Board-provided ST7262 RGB LCD driver.
static Touch *g_touch = nullptr;                  // Board-provided GT911 touch driver.
// Generated resource is staged by target CI. Montserrat retains LVGL icon glyphs.
#if __has_include("flytec_bg_14.generated.h")
#include "flytec_bg_14.generated.h"
LV_FONT_DECLARE(flytec_bg_14);
static lv_font_t flytecUiFont;
#endif
static lv_display_t *g_display = nullptr;         // LVGL 9 display instance.
static lv_indev_t *g_touch_input = nullptr;       // LVGL 9 pointer input device.
static void *g_draw_buffer = nullptr;             // Partial rendering buffer allocated in PSRAM.
static bool g_display_ready = false;              // True only after all display initialisation succeeds.
static bool g_sd_ready = false;                    // True only when the onboard TF card mounts.
static uint32_t g_last_handler_ms = 0;            // Used to keep LVGL handler calls paced.

static constexpr uint32_t LCD_WIDTH = 800;         // Native horizontal resolution of this board.
static constexpr uint32_t LCD_HEIGHT = 480;       // Native vertical resolution of this board.
static constexpr uint32_t BUFFER_LINES = 40;      // Partial buffer height; keeps RAM use moderate.
static constexpr uint32_t BYTES_PER_PIXEL = 2;    // RGB565 uses 16 bits (2 bytes) per pixel.

// LVGL asks for the current time in milliseconds through this callback.
static uint32_t displayPort_getTick()
{
    return millis();                               // Arduino millis() is the LVGL tick source.
}

// Copy one rendered LVGL rectangle to the Waveshare RGB panel.
static void displayPort_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    // The LVGL callback should always provide a display, but do not dereference a null one.
    if (display == nullptr)
    {
        return;                                    // There is no display handle to signal.
    }

    if (g_lcd == nullptr || area == nullptr || pixels == nullptr) // Reject invalid flush requests.
    {
        lv_display_flush_ready(display);           // Release LVGL's buffer even on an invalid request.
        return;                                    // Nothing can be drawn without valid arguments.
    }

    const int32_t width = area->x2 - area->x1 + 1; // LVGL area end coordinates are inclusive.
    const int32_t height = area->y2 - area->y1 + 1;// Calculate the inclusive rectangle height.

    if (width <= 0 || height <= 0)                // Ignore malformed or empty rectangles.
    {
        lv_display_flush_ready(display);           // Tell LVGL it may reuse the buffer.
        return;                                    // No panel transfer is required.
    }

    const bool copied = g_lcd->drawBitmap(         // Waveshare panel driver copies this RGB565 rectangle.
        area->x1, area->y1, width, height, pixels);// Use the exact rectangle and LVGL pixel buffer.

    if (!copied)                                   // Check whether the panel driver accepted the transfer.
    {
        Serial.println("DISPLAY: LCD drawBitmap failed."); // Report the display transfer failure.
    }

    // The RGB panel path copies the supplied bitmap into its frame buffer before returning.
    lv_display_flush_ready(display);               // Release LVGL's partial buffer after the copy.
}

// Read one GT911 touch point and report it to LVGL.
static void displayPort_readTouch(lv_indev_t *input, lv_indev_data_t *data)
{
    (void)input;                                   // The adapter uses the single global GT911 instance.

    if (g_touch == nullptr || data == nullptr)     // Guard against an unavailable touch controller.
    {
        if (data != nullptr)                       // Only write the state when LVGL supplied a data object.
        {
            data->state = LV_INDEV_STATE_RELEASED; // Report no touch while the controller is unavailable.
        }
        return;                                    // There is no valid touch point to report.
    }

    TouchPoint point;                              // Storage for the first touch point from the GT911.
    const int count = g_touch->readPoints(&point, 1, 0); // Poll once without waiting for an interrupt.

    if (count > 0)                                 // A touch point was reported by the controller.
    {
        data->point.x = point.x;                   // Forward the horizontal touch coordinate.
        data->point.y = point.y;                   // Forward the vertical touch coordinate.
        data->state = LV_INDEV_STATE_PRESSED;      // Tell LVGL that the pointer is pressed.
    }
    else                                           // No point is currently being touched.
    {
        data->state = LV_INDEV_STATE_RELEASED;     // Tell LVGL that the pointer is released.
    }
}

// Mount the onboard TF card after Board::begin() has initialised the shared CH422G expander.
static bool displayPort_mountSD()
{
    if (g_board == nullptr || g_board->getIO_Expander() == nullptr) // The board must own an initialised expander.
    {
        Serial.println("SD: Board I/O expander is unavailable."); // Explain why SD cannot be started.
        return false;                              // Keep the display usable without storage.
    }

    auto *expander = g_board->getIO_Expander()->getBase(); // Reuse the board's existing CH422G instance.
    if (expander == nullptr)                       // Verify that the low-level expander exists.
    {
        Serial.println("SD: CH422G base driver is unavailable."); // Report the missing driver.
        return false;                              // Do not create a second I2C expander.
    }

    expander->digitalWrite(4, LOW);                 // Assert physical TF-card CS via CH422G EXIO4 (active low).
    pinMode(6, OUTPUT);                            // GPIO6 is an unused dummy CS for Arduino's SD library.
    digitalWrite(6, HIGH);                         // Keep the dummy pin inactive; real CS is EXIO4.
    SPI.setHwCs(false);                            // Disable hardware-controlled CS on this SPI bus.
    SPI.begin(12, 13, 11, -1);                     // Configure SCK=12, MISO=13, MOSI=11, no native CS.

    // The SD library still requires a GPIO CS argument; GPIO6 is only a dummy.
    // The physical card remains selected through CH422G EXIO4, as required by Waveshare.
    if (!SD.begin(6, SPI, 4000000))                 // Mount the card at a conservative 4 MHz SPI clock.
    {
        Serial.println("SD: TF card mount failed or no card is inserted."); // Storage is optional at startup.
        return false;                              // Continue running the display without map storage.
    }

    if (SD.cardType() == CARD_NONE)                 // Verify that a card was actually detected.
    {
        Serial.println("SD: No TF card detected."); // Report an empty card slot.
        SD.end();                                   // Release the filesystem mount.
        return false;                              // Keep map functions disabled.
    }

    Serial.printf("SD: TF card ready, %llu MB.\n", SD.cardSize() / (1024ULL * 1024ULL)); // Log card capacity.
    return true;                                    // The SD filesystem is ready for map tile reads.
}

// Initialise the Waveshare board, LCD, touch controller and LVGL 9 port.
bool displayPort_begin()
{
    g_display_ready = false;                       // Start in a safe not-ready state.

    Serial.println("DISPLAY: Initialising Waveshare board..."); // Announce hardware startup.

    g_board = new Board();                         // Create the board object using its standard configuration.
    if (g_board == nullptr)                        // Check whether allocation succeeded.
    {
        Serial.println("DISPLAY: Board allocation failed."); // Explain the failure on the serial console.
        return false;                              // Do not create any LVGL objects.
    }

    g_board->init();                               // Configure board peripherals and the CH422G expander.
    if (!g_board->begin())                         // Start the LCD and touch devices using board configuration.
    {
        Serial.println("DISPLAY: Board begin failed."); // Report board startup failure.
        return false;                              // Keep the UI disabled.
    }

    g_sd_ready = displayPort_mountSD();             // Mount optional TF storage using the board-owned CH422G.

    g_lcd = g_board->getLCD();                     // Obtain the initialised ST7262 LCD driver.
    g_touch = g_board->getTouch();                 // Obtain the initialised GT911 touch driver.

    if (g_lcd == nullptr || g_touch == nullptr)    // Both hardware devices are required for this remote UI.
    {
        Serial.println("DISPLAY: LCD or touch device is unavailable."); // Report missing hardware object.
        return false;                              // Do not register an incomplete display port.
    }

    if (g_lcd->getFrameWidth() != LCD_WIDTH ||     // Verify the panel's reported horizontal resolution.
        g_lcd->getFrameHeight() != LCD_HEIGHT)    // Verify the panel's reported vertical resolution.
    {
        Serial.printf("DISPLAY: Unexpected LCD size %d x %d.\n", // Print the actual reported dimensions.
                      g_lcd->getFrameWidth(), g_lcd->getFrameHeight());
        return false;                              // Avoid drawing with incorrect geometry assumptions.
    }

    lv_init();                                     // Initialise LVGL exactly once for this application.
    lv_tick_set_cb(displayPort_getTick);           // Use Arduino millis() as the LVGL time source.

    const uint32_t buffer_bytes = LCD_WIDTH * BUFFER_LINES * BYTES_PER_PIXEL; // Size of one 40-row RGB565 buffer.
    g_draw_buffer = heap_caps_malloc(              // Allocate the partial rendering buffer from PSRAM.
        buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); // Require byte-addressable external RAM.

    if (g_draw_buffer == nullptr)                  // Verify that the PSRAM allocation succeeded.
    {
        Serial.println("DISPLAY: Could not allocate LVGL draw buffer in PSRAM."); // Report memory failure.
        return false;                              // No display can be registered without a draw buffer.
    }

    g_display = lv_display_create(LCD_WIDTH, LCD_HEIGHT); // Create the LVGL 9 display at 800 x 480.
    if (g_display == nullptr)                      // Check the LVGL display allocation.
    {
        Serial.println("DISPLAY: lv_display_create failed."); // Report LVGL registration failure.
        return false;                              // Leave the screen disabled.
    }

#if defined(FLYTEC_BG_FONT_GENERATED)
    flytecUiFont = flytec_bg_14;
    flytecUiFont.fallback = &lv_font_montserrat_14;
    lv_display_set_theme(g_display, lv_theme_default_init(g_display,
        lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_CYAN), true, &flytecUiFont));
#else
    Serial.println("DISPLAY: Bulgarian font missing; generate the UI font before release.");
#endif
    lv_display_set_color_format(g_display, LV_COLOR_FORMAT_RGB565); // Match the panel's 16-bit RGB pixel format.
    lv_display_set_flush_cb(g_display, displayPort_flush); // Register the LVGL 9 panel flush callback.
    lv_display_set_buffers(                        // Register the partial draw buffer with LVGL.
        g_display, g_draw_buffer, nullptr, buffer_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL); // One PSRAM buffer is sufficient.

    g_touch_input = lv_indev_create();             // Create an LVGL 9 input device for the touch panel.
    if (g_touch_input == nullptr)                  // Check input-device allocation.
    {
        Serial.println("DISPLAY: lv_indev_create failed."); // Report touch registration failure.
        return false;                              // Do not report the display fully ready without touch.
    }

    lv_indev_set_type(g_touch_input, LV_INDEV_TYPE_POINTER); // Configure the input device as a touchscreen pointer.
    lv_indev_set_read_cb(g_touch_input, displayPort_readTouch); // Register the GT911 read callback.

    g_last_handler_ms = millis();                  // Initialise handler pacing timestamp.
    g_display_ready = true;                        // All required display and touch components are ready.

    Serial.println("DISPLAY: Waveshare LCD, GT911 and LVGL 9.6.x ready."); // Confirm successful initialisation.
    return true;                                   // Allow the application to create its LVGL screens.
}

// Service LVGL from the Arduino loop; no background LVGL task is created here.
void displayPort_loop()
{
    if (!g_display_ready)                          // Do nothing until display initialisation succeeds.
    {
        return;                                    // Keep the rest of the firmware running safely.
    }

    const uint32_t now = millis();                 // Read the current Arduino millisecond counter.
    if (now - g_last_handler_ms >= 5)              // Service LVGL approximately every five milliseconds.
    {
        g_last_handler_ms = now;                   // Save the timestamp for the next service interval.
        lv_timer_handler();                        // Run LVGL timers, input processing and screen refresh.
    }
}

// Tell the application whether it may safely create LVGL objects.
bool displayPort_isReady()
{
    return g_display_ready;                        // Return the final display initialisation state.
}

// Report whether the onboard TF card mounted successfully.
bool displayPort_sdReady()
{
    return g_sd_ready;                              // Return the independent storage status.
}

// Set the Waveshare backlight output through the already-initialised CH422G.
// The current board configuration uses an expander switch, so only on/off is supported.
bool displayPort_setBacklight(bool enabled)
{
    if (g_board == nullptr || g_board->getIO_Expander() == nullptr)
        return false;
    auto *expander = g_board->getIO_Expander()->getBase();
    if (expander == nullptr)
        return false;
    expander->digitalWrite(2, enabled ? HIGH : LOW);
    return true;
}
