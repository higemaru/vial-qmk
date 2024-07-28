/* SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

        [0] = LAYOUT(
            KC_NO, KC_RIGHT, KC_UP, KC_LEFT, KC_DOWN, KC_NO
        ),
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] =   {
        ENCODER_CCW_CW(KC_PAGE_UP, KC_PAGE_DOWN),
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU)
    }
};
#endif
