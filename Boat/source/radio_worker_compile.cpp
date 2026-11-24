#include "radio_worker.h"
#include "radio_watchdog.h"
#include <type_traits>
static_assert(!std::is_copy_constructible<flytec_rtos::RadioWorker>::value,"Single owner required");
