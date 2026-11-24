#include "radio_worker.h"
#include "radio_watchdog.h"
#include <type_traits>
static_assert(!std::is_copy_constructible<flytec_rtos::RadioWorker>::value,"Single owner required");
#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
#include "tx_generation_gate.h"
#include "i2c_bus_guard.h"
#include "nrf_phy_adapter.h"
#include "sc18is606_nrf_link.h"
template class flytec_rtos::NrfPhyAdapter<Sc18is606NrfLink>;
#include "e22_phy_adapter.h"
#include "e22_uart_link.h"
template class flytec_rtos::E22PhyAdapter<E22UartLink>;
#include "dual_phy_worker.h"
using DiagnosticDualPoll=flytec_rtos::DualPhyPoll<flytec_rtos::NrfPhyAdapter<Sc18is606NrfLink>,flytec_rtos::E22PhyAdapter<E22UartLink>>;
template class flytec_rtos::DualPhyPoll<flytec_rtos::NrfPhyAdapter<Sc18is606NrfLink>,flytec_rtos::E22PhyAdapter<E22UartLink>>;
template class flytec_rtos::DualPhyWorker<DiagnosticDualPoll>;
