#pragma once
// Pure UI text mapping. TF card present != offline map pack installed.
// No LVGL, SD, GPIO, heap allocation or filesystem side effects.
namespace flytec_ui {
inline const char *sdStorageStatus(bool sdCardMounted, bool offlineMapInstalled) {
    if (!sdCardMounted) return "НЯМА SD КАРТА";
    return offlineMapInstalled ? "SD КАРТА: ГОТОВА" : "НЯМА ОФЛАЙН КАРТА";
}
} // namespace flytec_ui
