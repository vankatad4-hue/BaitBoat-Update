// Boat PlatformIO adapter ONLY: compile the existing Boat Arduino .ino verbatim.
// Legacy Arduino CLI stages *.cpp too, so keep this guarded to avoid
// duplicate setup()/loop() definitions in the official target workflow.
#if defined(FLYTEC_PLATFORMIO_BUILD)
#include "firmware/Flytec_Boat.ino"
#endif
