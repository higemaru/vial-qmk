/* SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

        [0] = LAYOUT(
            KC_ESC,  KC_Q, KC_W, KC_E, KC_R, KC_T,                  KC_Y, KC_U, KC_I,    KC_O,   KC_P,    KC_RSFT,
            KC_TAB,  KC_A, KC_S, KC_D, KC_F, KC_G,                  KC_H, KC_J, KC_K,    KC_L,   KC_SCLN, KC_GRV,
            KC_LCTL, KC_Z, KC_X, KC_C, KC_V, KC_B,                  KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_QUOT,
            KC_LSFT, KC_LNG2, KC_LALT, KC_LGUI, LT(1,KC_SPC), LT(2,KC_SPC), KC_ENT,  KC_BSPC,KC_LNG1, KC_MINS
        ),

        [1] = LAYOUT(
            KC_TRNS, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC,   KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_TRNS,
            KC_TRNS, KC_1,    KC_2,  KC_3,    KC_4,   KC_5,      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_MINS, KC_EQL,  KC_LBRC, KC_RBRC, KC_TRNS, KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  MO(3), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
        ),

        [2] = LAYOUT(
            KC_TRNS, KC_F2,   KC_F3,   KC_F4,   KC_F5, KC_F6,  KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_F11,  KC_TRNS,
            KC_F1,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT, KC_TRNS,   KC_F12,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS,  KC_UNDS, KC_PLUS, KC_LCBR, KC_RCBR,  KC_BSLS, KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MO(3),  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
        ),


        [3] = LAYOUT(
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
            KC_TRNS, KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS,   KC_TRNS,
            KC_TRNS, KC_BRID, KC_BRIU, KC_TRNS,   KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
        ),

};
