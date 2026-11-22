undefined
## LVGL / PlatformIO compatibility (R30H-10 software pass)

The non-actuating, explicitly labeled DEMO sonar illustration in `boat_ui.cpp` uses integer LVGL 9.6 point coordinates instead of narrowing float initializers. This repairs the known Remote PlatformIO compiler error without changing sonar, timing, STOP/RTH, radio security, or source-of-truth Arduino behavior. A complete fresh PlatformIO and Arduino target CI result is required before calling either build verified.
