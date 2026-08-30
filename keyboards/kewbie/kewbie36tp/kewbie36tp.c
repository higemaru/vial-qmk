/* SPDX-License-Identifier: GPL-2.0-or-later */

/* ============================================================
 * Kewbie36tp — キーボードレベルの実装
 *
 * トラックパッド（TPS43 / Azoteq IQS572）に関する処理はすべてここ。
 * キーマップを差し替えても挙動が変わらないよう _kb 層に置いている。
 *
 * ref. https://74th.hateblo.jp/entry/testing-TPS43-201A-S
 * ============================================================ */

#include "quantum.h"
#include <stdlib.h>
#include <string.h>

#include "custom_keycodes.h"

/* ---- グローバルキャッシュ ---- */
static int8_t  g_tp_multiplier      = 1;   // +1 or -1
static uint8_t g_tp_speed           = TP_SPEED_DEFAULT;
static bool    g_tp_enabled         = true;
static bool    g_tp_inertia_enabled = true;

/* ---- 慣性スクロール設定 ---- */
#define INERTIA_SCALE       256   // 固定小数点スケール
#define INERTIA_DECAY       235   // 毎フレーム保持率 (235/256 ≒ 92%)。大きいほど長く滑る
#define INERTIA_MIN_VEL     (8 * INERTIA_SCALE)  // これ未満で停止
#define INERTIA_SMOOTH_NUM  3     // スムージング係数（大きいほど直近重視）
#define INERTIA_BOOST       160   // 慣性開始時のブースト率(%) 狭いパッド対策で増幅
/*
 * 弱いフリックで滑らない       → INERTIA_BOOST を 180〜200 まで上げる
 * ちょっと触るだけで暴走する   → INERTIA_MIN_VEL を上げる、または INERTIA_SMOOTH_NUM を大きくする
 * 滑りが短すぎる               → INERTIA_DECAY を 240〜250 あたりまで上げる
 */

static int32_t g_inertia_vel_h = 0;   // 慣性用の速度（raw値 × INERTIA_SCALE）
static int32_t g_inertia_vel_v = 0;

static int32_t scroll_amount_h = 0;
static int32_t scroll_amount_v = 0;

/* ---- 速度テーブル（体感差を均等に） ----
 * speed 1〜10 を /100 の分母で表現。小さいほど速い。 */
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

/* ============================================================
 * EEPROM
 * ============================================================ */

void tp_cache_load(void) {
    uint8_t inv   = eeprom_read_byte((uint8_t *)(EECONFIG_USER + EEP_TP_INVERT));
    uint8_t speed = eeprom_read_byte((uint8_t *)(EECONFIG_USER + EEP_TP_SPEED));
    uint8_t en    = eeprom_read_byte((uint8_t *)(EECONFIG_USER + EEP_TP_EN));

    g_tp_multiplier = (inv == 1) ? -1 : 1;
    g_tp_speed      = (speed == 0 || speed > 10) ? TP_SPEED_DEFAULT : speed;
    g_tp_enabled    = (en != 0xFF && en != 0x00) ? (en == 1) : true;  // 未初期化ならデフォルト ON
}

void keyboard_post_init_kb(void) {
    tp_cache_load();

    keyboard_post_init_user();
}

/* ファームウェア初回書き込み時などに、EEPROM のデフォルト値を設定する */
void eeconfig_init_kb(void) {
    // デフォルトのスクロール方向は Mac（ナチュラルスクロール）
    uint8_t default_invert = 1;

    // OS判別機能が有効な場合、Windows と判定できたときだけ従来方向にする
#ifdef OS_DETECTION_ENABLE
    if (detected_host_os() == OS_WINDOWS) {
        default_invert = 0;
    }
#endif

    eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_INVERT), default_invert);
    eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_SPEED), TP_SPEED_DEFAULT);
    eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_EN), 1);  // デフォルトは有効

    eeconfig_init_user();
}

/* ============================================================
 * カスタムキーコード
 * ============================================================ */

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {  // キーが押された時だけ反応
        switch (keycode) {
            case TP_TOGG_INV: {
                uint8_t inv = eeprom_read_byte((uint8_t *)(EECONFIG_USER + EEP_TP_INVERT));
                inv         = !inv;
                eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_INVERT), inv);
                g_tp_multiplier = (inv == 1) ? -1 : 1;  // キャッシュ更新
                return false;                           // 他のキー処理を中断
            }
            case TP_TOGG_EN: {
                g_tp_enabled = !g_tp_enabled;
                eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_EN), g_tp_enabled ? 1 : 0);
                return false;
            }
            case TP_SPEED_INC: {
                if (g_tp_speed < 10) {
                    g_tp_speed++;
                    eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_SPEED), g_tp_speed);
                }
                return false;
            }
            case TP_SPEED_DEC: {
                if (g_tp_speed > 1) {
                    g_tp_speed--;
                    eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_SPEED), g_tp_speed);
                }
                return false;
            }
            case TP_SPEED_RST: {
                g_tp_speed = TP_SPEED_DEFAULT;
                eeprom_update_byte((uint8_t *)(EECONFIG_USER + EEP_TP_SPEED), TP_SPEED_DEFAULT);
                return false;
            }
            case TP_DRAG_LOCK: {
                static bool drag_locked = false;
                drag_locked             = !drag_locked;
                if (drag_locked) {
                    register_code(KC_BTN1);
                } else {
                    unregister_code(KC_BTN1);
                }
                return false;
            }
            case TP_INERTIA: {
                g_tp_inertia_enabled = !g_tp_inertia_enabled;
                if (!g_tp_inertia_enabled) {
                    g_inertia_vel_h = 0;
                    g_inertia_vel_v = 0;
                }
                return false;
            }
        }
    }

    return process_record_user(keycode, record);
}

/* ============================================================
 * ポインティングデバイス
 * ============================================================ */

report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    if (!g_tp_enabled) {
        memset(&mouse_report, 0, sizeof(mouse_report));
        return pointing_device_task_user(mouse_report);
    }

    int32_t div       = speed_divisor[g_tp_speed];
    bool    has_input = (mouse_report.h != 0 || mouse_report.v != 0);
    int32_t raw_h = 0, raw_v = 0;

    if (has_input) {
        raw_h = (int32_t)mouse_report.h * g_tp_multiplier;
        raw_v = (int32_t)mouse_report.v * g_tp_multiplier;

        // 慣性用速度をスムージングして更新
        g_inertia_vel_h = (g_inertia_vel_h * (INERTIA_SMOOTH_NUM - 1) + raw_h * INERTIA_SCALE) / INERTIA_SMOOTH_NUM;
        g_inertia_vel_v = (g_inertia_vel_v * (INERTIA_SMOOTH_NUM - 1) + raw_v * INERTIA_SCALE) / INERTIA_SMOOTH_NUM;

    } else if (g_tp_inertia_enabled && (g_inertia_vel_h != 0 || g_inertia_vel_v != 0)) {
        // 慣性フェーズ：保持速度をブーストして仮想入力として注入
        raw_h = (g_inertia_vel_h * INERTIA_BOOST / 100) / INERTIA_SCALE;
        raw_v = (g_inertia_vel_v * INERTIA_BOOST / 100) / INERTIA_SCALE;

        g_inertia_vel_h = g_inertia_vel_h * INERTIA_DECAY / 256;
        g_inertia_vel_v = g_inertia_vel_v * INERTIA_DECAY / 256;

        if (abs(g_inertia_vel_h) < INERTIA_MIN_VEL) g_inertia_vel_h = 0;
        if (abs(g_inertia_vel_v) < INERTIA_MIN_VEL) g_inertia_vel_v = 0;
    }

    // タッチ中・慣性中どちらも同じ蓄積 → div 変換ロジックを通す
    scroll_amount_h += raw_h;
    scroll_amount_v += raw_v;

    int8_t h = scroll_amount_h / div;
    int8_t v = scroll_amount_v / div;
    scroll_amount_h -= h * div;
    scroll_amount_v -= v * div;

    mouse_report.h = h;
    mouse_report.v = v;

    return pointing_device_task_user(mouse_report);
}

/* ------------------------------------------------------------
 * 【不採用】RDY（GP21）によるゲート — 新基板向けの検討結果
 *
 * テスト基板では FPC1 pin1 (RDY) を GP21 に配線したが、この層で
 * ゲートすることはできない。理由は 2 つ。
 *
 * 1) 慣性スクロールが止まる
 *    上の慣性フェーズは has_input == false のときに走る減衰ループで、
 *    指を離した後も毎サイクル呼ばれ続けることが前提。RDY が Low の
 *    ときに早期 return すると、指を離した瞬間に慣性が発動しない。
 *
 * 2) I2C トラフィックが減らない
 *    QMK は get_report()（I2C リード）→ pointing_device_task_kb() の
 *    順で呼ぶ。ここでゲートしてもリードは既に終わっている。
 *
 * POINTING_DEVICE_MOTION_PIN も active-low 前提で、IQS5xx の RDY は
 * active-high なので極性が逆。そのままでは使えない。
 *
 * ゲートするなら場所はドライバ層の get_report() の手前。自前ドライバを
 * 書くとき、または低消費電力モードを実装するときに再検討する。
 *
 * report_mouse_t pointing_device_task_kb(report_mouse_t report) {
 *     if (!gpio_read_pin(TPS43_RDY_PIN)) {
 *         return report;
 *     }
 *     return pointing_device_task_user(report);
 * }
 * ------------------------------------------------------------ */

/* ------------------------------------------------------------
 * 新基板（RP2040 直載せ）へ移行するときに有効化する
 *
 * TPS43 の RST を GP20 に配線するため、ポインティングデバイスの
 * 初期化より前に High にする必要がある。RP2040 の GPIO はリセット
 * 直後プルダウン入力なので、放置するとリセット保持のままになる。
 *
 * void keyboard_pre_init_kb(void) {
 *     gpio_set_pin_output(TPS43_RST_PIN);
 *     gpio_write_pin_high(TPS43_RST_PIN);
 *     wait_ms(50);              // IQS572 の起動待ち
 *
 *     keyboard_pre_init_user();
 * }
 * ------------------------------------------------------------ */
