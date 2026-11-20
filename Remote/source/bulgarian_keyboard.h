#pragma once
#include <lvgl.h>
// Phonetic QWERTY-style Bulgarian. Additional Cyrillic keys occupy row ends.
static const char * const bgLower[]={
    "я","в","е","р","т","ъ","у","и","о","п","ш","щ","\n",
    "а","с","д","ф","г","х","й","к","л","ч","\n",
    "з","ь","ц","ж","б","н","м","ю","\n",
    LV_SYMBOL_BACKSPACE,LV_SYMBOL_LEFT," ",LV_SYMBOL_RIGHT,LV_SYMBOL_OK,""
};
static const char * const bgUpper[]={
    "Я","В","Е","Р","Т","Ъ","У","И","О","П","Ш","Щ","\n",
    "А","С","Д","Ф","Г","Х","Й","К","Л","Ч","\n",
    "З","Ь","Ц","Ж","Б","Н","М","Ю","\n",
    LV_SYMBOL_BACKSPACE,LV_SYMBOL_LEFT," ",LV_SYMBOL_RIGHT,LV_SYMBOL_OK,""
};
static lv_buttonmatrix_ctrl_t bgCtrl[35];
inline void flytecInstallBulgarianKeyboard(lv_obj_t *keyboard){
 for(auto &ctrl:bgCtrl)ctrl=static_cast<lv_buttonmatrix_ctrl_t>(1);
 bgCtrl[32]=static_cast<lv_buttonmatrix_ctrl_t>(6);
 lv_keyboard_set_map(keyboard,LV_KEYBOARD_MODE_USER_1,bgLower,bgCtrl);
 lv_keyboard_set_map(keyboard,LV_KEYBOARD_MODE_USER_2,bgUpper,bgCtrl);
}
inline lv_keyboard_mode_t flytecKeyboardMode(uint8_t layout){return layout==0?LV_KEYBOARD_MODE_TEXT_LOWER:layout==1?LV_KEYBOARD_MODE_USER_1:LV_KEYBOARD_MODE_USER_2;}
inline const char *flytecKeyboardNextLabel(uint8_t layout){return layout==0?"EN / БГ":layout==1?"БГ / БГ↑":"БГ↑ / EN";}

