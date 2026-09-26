/* SPDX-License-Identifier: GPL-2.0-or-later */

/* ============================================================
 * TPS43（Azoteq IQS572）用 自前ポインティングドライバ
 *
 * QMK 標準の azoteq_iqs5xx ドライバは 0x000C から 10 バイトしか読まず、
 * 絶対座標（0x0016〜）を捨てている。IQS5xx は 0xEEEE への書き込みで
 * 通信窓を閉じると次の計測周期まで応答しないため、後から追加で読むことも
 * できない。そこで 1 回の通信で 14 バイト読む。
 *
 *   0x000C       前回の処理時間
 *   0x000D       ジェスチャー 0（1本指）
 *   0x000E       ジェスチャー 1（複数本指）
 *   0x000F-10    システム情報
 *   0x0011       指の本数
 *   0x0012-13    相対 X（ビッグエンディアン、符号付き）
 *   0x0014-15    相対 Y
 *   0x0016-17    1本目の指の絶対 X（ビッグエンディアン）
 *   0x0018-19    1本目の指の絶対 Y
 *
 * レイアウトは Linux の drivers/input/touchscreen/iqs5xx.c と一致を確認済み。
 *
 * QMK / vial-qmk のバージョン差（関数名・戻り値・マクロの有無）に左右されない
 * よう、初期化も含めて QMK の azoteq_iqs5xx.c には依存しない。
 * 初期化手順と既定値は QMK の azoteq_iqs5xx.c（GPL-2.0-or-later）を移植したもの。
 * 設定は QMK と同じ AZOTEQ_IQS5XX_* の define で変えられる。
 * ============================================================ */

#include "quantum.h"
#include "i2c_master.h"
#include "pointing_device.h"

#include "tp_circular.h"

/* ---- デバッグ出力 ---- */
#ifndef pd_dprintf
#    define pd_dprintf(...) dprintf(__VA_ARGS__)
#endif

/* ---- I2C ---- */
#ifndef AZOTEQ_IQS5XX_ADDRESS
#    define AZOTEQ_IQS5XX_ADDRESS (0x74 << 1)
#endif
#ifndef AZOTEQ_IQS5XX_TIMEOUT_MS
#    define AZOTEQ_IQS5XX_TIMEOUT_MS 10
#endif
#ifndef AZOTEQ_IQS5XX_REPORT_RATE
#    define AZOTEQ_IQS5XX_REPORT_RATE 10
#endif

/* ---- レジスタ ---- */
#define REG_PRODUCT_NUMBER 0x0000
#define REG_BASE_DATA 0x000C
#define REG_SYSTEM_CONTROL_1 0x0432
#define REG_REPORT_RATE_ACTIVE 0x057A // +2: IDLE_TOUCH, +4: IDLE
#define REG_IDLE_MODE_TIMEOUT 0x0586
#define REG_SYSTEM_CONFIG_0 0x058E
#define REG_SYSTEM_CONFIG_1 0x058F
#define REG_XY_CONFIG_0 0x0669
#define REG_X_RESOLUTION 0x066E // X(2) + Y(2)、ビッグエンディアン
#define REG_GESTURE_CONFIG 0x06B7 // 24 バイト
#define REG_END_COMMS 0xEEEE

/* ---- パッド（TPS43） ---- */
#define PAD_WIDTH_MM 43
#define PAD_HEIGHT_MM 40
#define PAD_MAX_RES_X 2048
#define PAD_MAX_RES_Y 1792

/* ---- ジェスチャーの既定値（QMK と同じ） ---- */
#ifndef AZOTEQ_IQS5XX_TAP_ENABLE
#    define AZOTEQ_IQS5XX_TAP_ENABLE true
#endif
#ifndef AZOTEQ_IQS5XX_PRESS_AND_HOLD_ENABLE
#    define AZOTEQ_IQS5XX_PRESS_AND_HOLD_ENABLE false
#endif
#ifndef AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE
#    define AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE true
#endif
#ifndef AZOTEQ_IQS5XX_SCROLL_ENABLE
#    define AZOTEQ_IQS5XX_SCROLL_ENABLE true
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_X_ENABLE
#    define AZOTEQ_IQS5XX_SWIPE_X_ENABLE false
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_Y_ENABLE
#    define AZOTEQ_IQS5XX_SWIPE_Y_ENABLE false
#endif
#ifndef AZOTEQ_IQS5XX_ZOOM_ENABLE
#    define AZOTEQ_IQS5XX_ZOOM_ENABLE false
#endif
#ifndef AZOTEQ_IQS5XX_TAP_TIME
#    define AZOTEQ_IQS5XX_TAP_TIME 0x96
#endif
#ifndef AZOTEQ_IQS5XX_TAP_DISTANCE
#    define AZOTEQ_IQS5XX_TAP_DISTANCE 0x19
#endif
#ifndef AZOTEQ_IQS5XX_HOLD_TIME
#    define AZOTEQ_IQS5XX_HOLD_TIME 0x12C
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_INITIAL_TIME
#    define AZOTEQ_IQS5XX_SWIPE_INITIAL_TIME 0x64
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_INITIAL_DISTANCE
#    define AZOTEQ_IQS5XX_SWIPE_INITIAL_DISTANCE 0x12C
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_TIME
#    define AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_TIME 0x0
#endif
#ifndef AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_DISTANCE
#    define AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_DISTANCE 0x7D0
#endif
#ifndef AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE
#    define AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE 0x32
#endif
#ifndef AZOTEQ_IQS5XX_ZOOM_INITIAL_DISTANCE
#    define AZOTEQ_IQS5XX_ZOOM_INITIAL_DISTANCE 0x32
#endif
#ifndef AZOTEQ_IQS5XX_ZOOM_CONSECUTIVE_DISTANCE
#    define AZOTEQ_IQS5XX_ZOOM_CONSECUTIVE_DISTANCE 0x19
#endif

/* ---- 円周スクロールの向き（時計回りで下スクロールなら 0） ---- */
#ifndef TP_CIRC_INVERT
#    define TP_CIRC_INVERT 0
#endif

/* ---- HID レポートの範囲 ---- */
#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
#define CLAMP_WHEEL(v) CLAMP((v), INT8_MIN, INT8_MAX)
#ifdef MOUSE_EXTENDED_REPORT
#    define CLAMP_XY(v) CLAMP((v), INT16_MIN, INT16_MAX)
#else
#    define CLAMP_XY(v) CLAMP((v), INT8_MIN, INT8_MAX)
#endif

#define BE16(p) ((uint16_t)(((uint16_t)(p)[0] << 8) | (p)[1]))

/* ジェスチャー 0（0x000D）のビット */
#define G0_SINGLE_TAP (1 << 0)
#define G0_PRESS_HOLD (1 << 1)
#define G0_SWIPE_X_NEG (1 << 2)
#define G0_SWIPE_X_POS (1 << 3)
#define G0_SWIPE_Y_POS (1 << 4)
#define G0_SWIPE_Y_NEG (1 << 5)
/* ジェスチャー 1（0x000E）のビット */
#define G1_TWO_FINGER_TAP (1 << 0)
#define G1_SCROLL (1 << 1)
#define G1_ZOOM (1 << 2)

static bool     g_ready           = false;
static bool     g_need_resolution = true;
static uint16_t g_res_x           = PAD_MAX_RES_X;

/* ============================================================
 * I2C の小物
 * ============================================================ */

static i2c_status_t rd(uint16_t reg, uint8_t *buf, uint16_t len) {
    return i2c_read_register16(AZOTEQ_IQS5XX_ADDRESS, reg, buf, len, AZOTEQ_IQS5XX_TIMEOUT_MS);
}

static i2c_status_t wr(uint16_t reg, const uint8_t *buf, uint16_t len) {
    return i2c_write_register16(AZOTEQ_IQS5XX_ADDRESS, reg, buf, len, AZOTEQ_IQS5XX_TIMEOUT_MS);
}

static i2c_status_t end_session(void) {
    const uint8_t any = 1;
    return wr(REG_END_COMMS, &any, 1);
}

/* 1 バイトのレジスタのビットを書き換える */
static i2c_status_t update_bits(uint16_t reg, uint8_t mask, uint8_t value) {
    uint8_t      v = 0;
    i2c_status_t s = rd(reg, &v, 1);
    if (s != I2C_STATUS_SUCCESS) return s;
    v = (uint8_t)((v & ~mask) | (value & mask));
    return wr(reg, &v, 1);
}

/* 眠っている IQS5xx を起こす。最初の通信は NACK になるので結果は見ない */
static void wake(void) {
    uint8_t dummy;
    rd(REG_PRODUCT_NUMBER, &dummy, 1);
}

/* ============================================================
 * 初期化
 * ============================================================ */

static i2c_status_t set_report_rates(void) {
    const uint8_t rate[2] = {(uint8_t)(AZOTEQ_IQS5XX_REPORT_RATE >> 8), (uint8_t)(AZOTEQ_IQS5XX_REPORT_RATE & 0xFF)};
    i2c_status_t  s       = I2C_STATUS_SUCCESS;
    for (uint8_t mode = 0; mode < 3; mode++) { // ACTIVE, IDLE_TOUCH, IDLE
        s |= wr(REG_REPORT_RATE_ACTIVE + 2 * mode, rate, 2);
    }
    return s;
}

static i2c_status_t set_xy_config(void) {
    uint8_t      v = 0;
    i2c_status_t s = rd(REG_XY_CONFIG_0, &v, 1);
    if (s != I2C_STATUS_SUCCESS) return s;
    // bit0: flip_x, bit1: flip_y, bit2: switch_xy, bit3: palm_reject
#if defined(AZOTEQ_IQS5XX_ROTATION_90)
    v ^= (1 << 1) | (1 << 2);
#elif defined(AZOTEQ_IQS5XX_ROTATION_180)
    v ^= (1 << 0) | (1 << 1);
#elif defined(AZOTEQ_IQS5XX_ROTATION_270)
    v ^= (1 << 0) | (1 << 2);
#endif
    v |= (1 << 3);
    return wr(REG_XY_CONFIG_0, &v, 1);
}

static void put_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFF);
}

static i2c_status_t set_gesture_config(void) {
    uint8_t      c[24] = {0};
    i2c_status_t s     = rd(REG_GESTURE_CONFIG, c, sizeof(c));
    if (s != I2C_STATUS_SUCCESS) return s;

    // 未使用ビット（c[0] の上位 2 ビット、c[1] の上位 5 ビット）は読んだ値を残す
    c[0] = (c[0] & 0xC0) | (AZOTEQ_IQS5XX_TAP_ENABLE ? G0_SINGLE_TAP : 0) | (AZOTEQ_IQS5XX_PRESS_AND_HOLD_ENABLE ? G0_PRESS_HOLD : 0) | (AZOTEQ_IQS5XX_SWIPE_X_ENABLE ? (G0_SWIPE_X_NEG | G0_SWIPE_X_POS) : 0) | (AZOTEQ_IQS5XX_SWIPE_Y_ENABLE ? (G0_SWIPE_Y_NEG | G0_SWIPE_Y_POS) : 0);
    c[1] = (c[1] & 0xF8) | (AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE ? G1_TWO_FINGER_TAP : 0) | (AZOTEQ_IQS5XX_SCROLL_ENABLE ? G1_SCROLL : 0) | (AZOTEQ_IQS5XX_ZOOM_ENABLE ? G1_ZOOM : 0);
    put_be16(&c[2], AZOTEQ_IQS5XX_TAP_TIME);
    put_be16(&c[4], AZOTEQ_IQS5XX_TAP_DISTANCE);
    put_be16(&c[6], AZOTEQ_IQS5XX_HOLD_TIME);
    put_be16(&c[8], AZOTEQ_IQS5XX_SWIPE_INITIAL_TIME);
    put_be16(&c[10], AZOTEQ_IQS5XX_SWIPE_INITIAL_DISTANCE);
    put_be16(&c[12], AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_TIME);
    put_be16(&c[14], AZOTEQ_IQS5XX_SWIPE_CONSECUTIVE_DISTANCE);
    // c[16]: swipe_angle はそのまま
    put_be16(&c[17], AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE);
    // c[19]: scroll_angle はそのまま
    put_be16(&c[20], AZOTEQ_IQS5XX_ZOOM_INITIAL_DISTANCE);
    put_be16(&c[22], AZOTEQ_IQS5XX_ZOOM_CONSECUTIVE_DISTANCE);

    return wr(REG_GESTURE_CONFIG, c, sizeof(c));
}

/* vial-qmk は void、upstream QMK は bool。
 * upstream でビルドするときは戻り値を bool にして g_ready を返すこと。 */
void pointing_device_driver_init(void) {
    i2c_init();

    // ソフトリセット（System Control 1 の bit1）
    wake();
    update_bits(REG_SYSTEM_CONTROL_1, (1 << 1) | (1 << 0), (1 << 1));
    end_session();
    wait_ms(100);

    wake();
    uint8_t pn[2] = {0};
    if (rd(REG_PRODUCT_NUMBER, pn, 2) != I2C_STATUS_SUCCESS || BE16(pn) == 0) {
        pd_dprintf("TPS43 - not found\n");
        g_ready = false;
        return;
    }
    pd_dprintf("TPS43 - product number %u\n", BE16(pn));

    i2c_status_t s = set_report_rates();
    const uint8_t no_timeout = 255; // LP1/LP2 に入らない
    s |= wr(REG_IDLE_MODE_TIMEOUT, &no_timeout, 1);
    // System Config 1: event_mode(bit0)=0、gesture(bit1)/tp(bit2)/touch(bit6)=1、その他=0
    s |= update_bits(REG_SYSTEM_CONFIG_1, 0xFF, (1 << 1) | (1 << 2) | (1 << 6));
    // System Config 0: reati(bit2)=1
    s |= update_bits(REG_SYSTEM_CONFIG_0, (1 << 2), (1 << 2));
    s |= set_xy_config();
    s |= set_gesture_config();
    end_session();
    wait_ms(AZOTEQ_IQS5XX_REPORT_RATE + 1);

    g_ready           = (s == I2C_STATUS_SUCCESS);
    g_need_resolution = true;
    pd_dprintf("TPS43 - init %s\n", g_ready ? "ok" : "failed");
}

/* ============================================================
 * CPI（= 解像度レジスタ）
 * ============================================================ */

#define DIV_ROUND(n, d) (((n) + ((d) / 2)) / (d))

uint16_t pointing_device_driver_get_cpi(void) {
    // 解像度 X を 1 インチあたりに換算
    return (uint16_t)DIV_ROUND((uint32_t)g_res_x * 254, PAD_WIDTH_MM * 10);
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    uint32_t rx = DIV_ROUND((uint32_t)cpi * PAD_WIDTH_MM * 10, 254);
    uint32_t ry = DIV_ROUND((uint32_t)cpi * PAD_HEIGHT_MM * 10, 254);
    if (rx > PAD_MAX_RES_X) rx = PAD_MAX_RES_X;
    if (ry > PAD_MAX_RES_Y) ry = PAD_MAX_RES_Y;

    uint8_t buf[4];
    put_be16(&buf[0], (uint16_t)rx);
    put_be16(&buf[2], (uint16_t)ry);
    wr(REG_X_RESOLUTION, buf, sizeof(buf));
    g_need_resolution = true; // 実際に効いた値を次の読み出しで確認する
}

/* 解像度レジスタを読む。通信窓の中で呼ぶこと */
static void read_resolution(void) {
    uint8_t buf[4];
    if (rd(REG_X_RESOLUTION, buf, sizeof(buf)) == I2C_STATUS_SUCCESS) {
        g_res_x = BE16(&buf[0]);
        tp_circ_set_resolution(BE16(&buf[0]), BE16(&buf[2]));
        g_need_resolution = false;
        pd_dprintf("TPS43 - resolution %u x %u\n", BE16(&buf[0]), BE16(&buf[2]));
    }
}

/* ============================================================
 * レポート
 * ============================================================ */

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
    report_mouse_t r = {0};
    (void)mouse_report;
    if (!g_ready) return r;

    uint8_t      d[14] = {0};
    i2c_status_t s     = rd(REG_BASE_DATA, d, sizeof(d));
    if (s != I2C_STATUS_SUCCESS) {
        pd_dprintf("TPS43 - read failed: %d\n", s);
        return r;
    }
    if (g_need_resolution) {
        read_resolution(); // 同じ通信窓の中で読む
    }
    end_session();

    const uint8_t  g0      = d[1];
    const uint8_t  g1      = d[2];
    const uint8_t  fingers = d[5];
    const int16_t  rel_x   = (int16_t)BE16(&d[6]);
    const int16_t  rel_y   = (int16_t)BE16(&d[8]);
    const uint16_t abs_x   = BE16(&d[10]);
    const uint16_t abs_y   = BE16(&d[12]);

#ifdef TP_CIRC_DEBUG
    if (fingers > 0) {
        uprintf("TPS43 f=%u abs=(%u,%u) rel=(%d,%d)\n", fingers, abs_x, abs_y, rel_x, rel_y);
    }
#endif

    /* ---- ジェスチャー（QMK 標準ドライバと同じ優先順位） ---- */
    bool ignore_movement = false;
    if (g0 & (G0_SINGLE_TAP | G0_PRESS_HOLD)) {
        r.buttons = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON1);
    } else if (g1 & G1_TWO_FINGER_TAP) {
        r.buttons = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON2);
    } else if (g0 & G0_SWIPE_X_NEG) {
        r.buttons       = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON4);
        ignore_movement = true;
    } else if (g0 & G0_SWIPE_X_POS) {
        r.buttons       = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON5);
        ignore_movement = true;
    } else if (g0 & G0_SWIPE_Y_NEG) {
        r.buttons       = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON6);
        ignore_movement = true;
    } else if (g0 & G0_SWIPE_Y_POS) {
        r.buttons       = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON3);
        ignore_movement = true;
    } else if (g1 & G1_ZOOM) {
        if (rel_x < 0) {
            r.buttons = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON7);
        } else if (rel_x > 0) {
            r.buttons = pointing_device_handle_buttons(r.buttons, true, POINTING_DEVICE_BUTTON8);
        }
    } else if (g1 & G1_SCROLL) {
        r.h = CLAMP_WHEEL(rel_x);
        r.v = CLAMP_WHEEL(rel_y);
    }

    /* ---- 円周スクロール ---- */
    int16_t circ = 0;
    if (tp_circ_update(fingers, abs_x, abs_y, &circ)) {
        // 円周スクロール中はカーソルを動かさない。
        // 時計回り = 下スクロール（HID の v は正が上）
        int32_t v = TP_CIRC_INVERT ? circ : -circ;
        r.v       = CLAMP_WHEEL(v);
        r.h       = 0;
        return r;
    }

    /* ---- 1本指のカーソル移動 ---- */
    if (fingers == 1 && !ignore_movement) {
        r.x = CLAMP_XY(rel_x);
        r.y = CLAMP_XY(rel_y);
    }

    return r;
}
