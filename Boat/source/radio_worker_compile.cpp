#include "radio_worker.h"
#include "radio_watchdog.h"
#include <type_traits>
static_assert(!std::is_copy_constructible<flytec_rtos::RadioWorker>::value,"Single owner required");
#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
#include "tx_generation_gate.h"
#include "nrf_phy_adapter.h"
#include "nrf24_link.h"
template class flytec_rtos::NrfPhyAdapter<NRF24Link>;
#include "e22_phy_adapter.h"
#include "e22_uart_board_port.h"
template class flytec_rtos::E22PhyAdapter<E22UartBoardPort>;
