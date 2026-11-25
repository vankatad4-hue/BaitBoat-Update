// R30H-88: compile-only coverage for the dormant dual-core diagnostic.
// No instance is constructed; no tasks, GPIO, radios or actuators are started.
#include "dual_core_diagnostic.h"
#include <type_traits>
static_assert(!std::is_copy_constructible<flytec_rtos::DualCoreDiagnostic>::value,
              "Diagnostic lifecycle must have a single owner");
static_assert(!std::is_copy_assignable<flytec_rtos::DualCoreDiagnostic>::value,
              "Diagnostic lifecycle must have a single owner");
