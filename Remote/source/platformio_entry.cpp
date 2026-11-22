// PlatformIO adapter ONLY: compile existing Waveshare Arduino sketch
// without moving or duplicating project logic. .ino is excluded from PIO
// build_src_filter; Arduino CLI builds the original .ino and excludes this.
#if defined(FLYTEC_PLATFORMIO_BUILD)
#include "Flytec_Remote.ino"
#endif
