/* SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "custom_keycodes.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

        [0] = LAYOUT(
            LGUI_T(KC_Q), KC_W, KC_E, KC_R, KC_T,                 KC_Y, KC_U, KC_I,    KC_O,   KC_P,
            CTL_T(KC_A), KC_S, KC_D, KC_F, KC_G,                  KC_H, KC_J, KC_K,    KC_L,   CTL_T(KC_SCLN),
            SFT_T(KC_Z), KC_X, KC_C, KC_V, KC_B,                  KC_N, KC_M, KC_COMM, KC_DOT, SFT_T(KC_MINS),
            LALT_T(KC_ESC), LGUI_T(KC_TAB), LT(1,KC_SPC), LT(2,KC_SPC), KC_ENT, CTL_T(KC_ESC)
        ),

        [1] = LAYOUT(
            KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC,   KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN,
            KC_1,    KC_2,  KC_3,    KC_4,   KC_5,      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,
            KC_TRNS, KC_EQL, KC_PLUS, KC_MINS, KC_GRV,          KC_QUOT,  KC_PIPE, KC_LBRC, KC_RBRC, KC_SLSH,
            KC_TRNS, KC_TRNS, KC_TRNS,  MO(3), KC_TRNS, KC_BSPC
        ),

        [2] = LAYOUT(
            EE_CLR, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT, KC_PIPE,
            KC_TRNS, KC_EQL,  KC_PLUS, KC_UNDS, KC_TILD,  KC_QUOT, KC_PIPE, KC_LCBR, KC_RCBR,  KC_BSLS,
            KC_TRNS, KC_TRNS, MO(3),  KC_TRNS, KC_TRNS, KC_BSPC
        ),


        [3] = LAYOUT(
            KC_F1, KC_F2,   KC_F3,   KC_F4,   KC_F5, KC_F6,  KC_F7,   KC_F8,   KC_F9,   KC_F10,
            KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_F11,
            KC_BRID, KC_BRIU, RGB_TOG,   KC_TRNS, KC_TRNS,  TP_TOGG_INV, KC_TRNS, KC_TRNS, KC_TRNS,  KC_F12,
            KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS
        ),

};

//ref. https://74th.hateblo.jp/entry/testing-TPS43-201A-S

// ファームウェア初回書き込み時などに、EEPROMのデフォルト値を設定する
void eeconfig_init_user(void) {
    // デフォルトのスクロール方向は Win
    uint8_t default_invert = 0;

    // OS判別機能が有効な場合
    #ifdef OS_DETECTION_ENABLE
        if (detected_host_os() != OS_WINDOWS) {
           default_invert = 1;
        }
    #endif

    // 判別したデフォルト値を書き込む
    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT), default_invert);
    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), 3);  // 速度は変更なし
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) { // キーが押された時だけ反応
        switch (keycode) {
            case TP_TOGG_INV: {
                uint8_t current_val = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT));
                eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT), !current_val); // 0と1を反転
                return false; // 他のキー処理を中断
            }
            case TP_SPEED_INC: {
                uint8_t speed = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED));
                if (speed < 10) { eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), speed + 1); }
                return false;
            }
            case TP_SPEED_DEC: {
                uint8_t speed = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED));
                if (speed > 1) { eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), speed - 1); }
                return false;
            }
            case TP_SPEED_RST: {
                eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), 3); // デフォルト値の3に戻す
                return false;
            }
        }
    }
    return true; // 上記以外のキーは通常通り処理
}

// EEPROMから設定を読み込み、タッチパッドの動作に反映させる
int32_t scroll_amount_h = 0;
int32_t scroll_amount_v = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // EEPROMから設定値を読み込む
    uint8_t invert_scroll = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT));
    uint8_t scroll_speed  = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED));

    if (scroll_speed == 0 || scroll_speed > 10) { // 安全装置
        scroll_speed = 3; // 不正な値ならデフォルト値にする
    }

    int8_t multiplier = (invert_scroll == 1) ? -1 : 1;

    scroll_amount_h += mouse_report.h * scroll_speed * multiplier;
    scroll_amount_v += mouse_report.v * scroll_speed * multiplier;

    int8_t h = scroll_amount_h / 100;
    int8_t v = scroll_amount_v / 100;
    scroll_amount_h -= h * 100;
    scroll_amount_v -= v * 100;

    mouse_report.h = h;
    mouse_report.v = v;
    return mouse_report;
}

/*

#define SCROLL_SCALE_PERCENT 3

int32_t scroll_amount_h = 0;
int32_t scroll_amount_v = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    #if defined(is_keyboard_master_os_windows)
    // Windowsの場合はそのまま
    scroll_amount_h += mouse_report.h * SCROLL_SCALE_PERCENT;
    scroll_amount_v += mouse_report.v * SCROLL_SCALE_PERCENT;
    #else
    // Windows以外（Macなど）の場合は反転
    scroll_amount_h -= mouse_report.h * SCROLL_SCALE_PERCENT;
    scroll_amount_v -= mouse_report.v * SCROLL_SCALE_PERCENT;
    #endif

    int8_t h = scroll_amount_h / 100;
    int8_t v = scroll_amount_v / 100;
    scroll_amount_h -= h * 100;
    scroll_amount_v -= v * 100;

    mouse_report.h = h;
    mouse_report.v = v;
    return mouse_report;
}
*/