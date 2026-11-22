#include "ui_sd_status.h"
#include <cassert>
#include <string>
int main() {
    using flytec_ui::sdStorageStatus;
    assert(std::string(sdStorageStatus(false, false)) == "НЯМА SD КАРТА");
    assert(std::string(sdStorageStatus(false, true)) == "НЯМА SD КАРТА");
    assert(std::string(sdStorageStatus(true, false)) == "НЯМА ОФЛАЙН КАРТА");
    assert(std::string(sdStorageStatus(true, true)) == "SD КАРТА: ГОТОВА");
    return 0;
}
