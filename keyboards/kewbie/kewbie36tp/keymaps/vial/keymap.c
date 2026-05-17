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
            KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS,    TP_TOGG_EN, KC_BTN1, KC_BTN2, TP_DRAG_LOCK , KC_F11,
            KC_BRID, KC_BRIU, RGB_TOG,   KC_TRNS, KC_TRNS,  TP_TOGG_INV, TP_SPEED_DEC, TP_SPEED_RST, TP_SPEED_INC,  KC_F12,
            KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS
        ),

};

//ref. https://74th.hateblo.jp/entry/testing-TPS43-201A-S

// --- グローバルキャッシュ ---
static int8_t  g_tp_multiplier = 1;   // +1 or -1
static uint8_t g_tp_speed      = TP_SPEED_DEFAULT;
static bool g_tp_enabled = true;

void tp_cache_load(void) {
    uint8_t inv   = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT));
    uint8_t speed = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED));
    uint8_t en = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_EN));

    g_tp_multiplier = (inv == 1) ? -1 : 1;
    g_tp_speed      = (speed == 0 || speed > 10) ? TP_SPEED_DEFAULT : speed;
    g_tp_enabled    = (en != 0xFF && en != 0x00) ? (en == 1) : true; // 未初期化ならデフォルトON
}

void keyboard_post_init_user(void) {
    tp_cache_load();
}

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
    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), TP_SPEED_DEFAULT);  // 速度は変更なし
    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_EN),     1); // デフォルトは有効
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) { // キーが押された時だけ反応
        switch (keycode) {
            case TP_TOGG_INV: {
                uint8_t inv = eeprom_read_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT));
                inv = !inv;
                eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_INVERT), inv); // 0と1を反転
                g_tp_multiplier = (inv == 1) ? -1 : 1;  // キャッシュ更新
                return false; // 他のキー処理を中断
            }
            case TP_TOGG_EN: {
                g_tp_enabled = !g_tp_enabled;
                eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_EN), g_tp_enabled ? 1 : 0);
                return false;
            }
            case TP_SPEED_INC: {
                if (g_tp_speed < 10) {
                    g_tp_speed++;
                    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), g_tp_speed);
                }
                return false;
            }
            case TP_SPEED_DEC: {
                if (g_tp_speed > 1) {
                    g_tp_speed--;
                    eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), g_tp_speed);
                }
                return false;
            }
            case TP_SPEED_RST: {
                g_tp_speed = TP_SPEED_DEFAULT;
                eeprom_update_byte((uint8_t*)(EECONFIG_USER + EEP_TP_SPEED), TP_SPEED_DEFAULT); // デフォルト値に戻す
                return false;
            }
            case TP_DRAG_LOCK: {
                static bool drag_locked = false;
                drag_locked = !drag_locked;
                if (drag_locked) {
                    register_code(KC_BTN1);
                } else {
                    unregister_code(KC_BTN1);
                }
                return false;
            }
        }
    }
    return true;
}

// --- 速度テーブル（体感差を均等に） ---
// speed 1〜10 を /100 の分母で表現
// 小さいほど速い
static const uint8_t speed_divisor[11] = {
    100,  // 0（未使用）
    200,  // 1: 最遅
    150,  // 2
    100,  // 3: デフォルト
     75,  // 4
     55,  // 5
     40,  // 6
     28,  // 7
     20,  // 8
     14,  // 9
     10,  // 10: 最速
};

// EEPROMから設定を読み込み、タッチパッドの動作に反映させる
static int32_t scroll_amount_h = 0;
static int32_t scroll_amount_v = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (!g_tp_enabled) {
        memset(&mouse_report, 0, sizeof(mouse_report));
        return mouse_report;
    }
    int32_t div = speed_divisor[g_tp_speed];

    scroll_amount_h += mouse_report.h * g_tp_multiplier;
    scroll_amount_v += mouse_report.v * g_tp_multiplier;

    int8_t h = scroll_amount_h / div;
    int8_t v = scroll_amount_v / div;
    scroll_amount_h -= h * div;
    scroll_amount_v -= v * div;

    mouse_report.h = h;
    mouse_report.v = v;
    return mouse_report;
}