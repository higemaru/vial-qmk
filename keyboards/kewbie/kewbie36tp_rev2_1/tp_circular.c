/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "tp_circular.h"

#include <math.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

/* 内部の長さ単位は 0.01mm。X と Y で 1mm あたりの解像度が違うので、
 * いったん mm 系にそろえてから角度と半径を計算する。 */
#define PAD_W_001MM ((int32_t)TP_PAD_WIDTH_MM * 100)
#define PAD_H_001MM ((int32_t)TP_PAD_HEIGHT_MM * 100)
#define RADIUS_001MM ((float)TP_CIRC_DIAMETER_MM * 50.0f)
#define RING_001MM (RADIUS_001MM * TP_CIRC_RING_PCT / 100.0f)
#define DEADZONE_001MM (RADIUS_001MM * TP_CIRC_DEADZONE_PCT / 100.0f)

static uint16_t g_res_x = 2048; // TPS43 の既定値。起動後にレジスタから読み直す
static uint16_t g_res_y = 1792;

static bool    g_active      = false;
static bool    g_angle_valid = false;
static float   g_prev_angle  = 0.0f;
static float   g_remainder   = 0.0f; // 出力しきれなかった端数
static uint8_t g_prev_fingers = 0;

void tp_circ_set_resolution(uint16_t res_x, uint16_t res_y) {
    if (res_x > 0 && res_y > 0) {
        g_res_x = res_x;
        g_res_y = res_y;
    }
}

bool tp_circ_is_active(void) {
    return g_active;
}

/* 絶対座標 → 円の中心からの相対位置（0.01mm） */
static void to_centered_001mm(uint16_t abs_x, uint16_t abs_y, float *dx, float *dy) {
    float x = (float)abs_x * PAD_W_001MM / g_res_x;
    float y = (float)abs_y * PAD_H_001MM / g_res_y;
    *dx     = x - (PAD_W_001MM / 2.0f + TP_CIRC_CENTER_OFFSET_X_01MM * 10.0f);
    *dy     = y - (PAD_H_001MM / 2.0f + TP_CIRC_CENTER_OFFSET_Y_01MM * 10.0f);
}

bool tp_circ_update(uint8_t fingers, uint16_t abs_x, uint16_t abs_y, bool engage, int16_t *scroll) {
    *scroll = 0;

#ifdef TP_CIRC_AUTO_RING
    const bool touch_down = (g_prev_fingers == 0 && fingers == 1);
#endif
    g_prev_fingers = fingers;

    if (fingers == 0) {
        // 指を全部離した → 終了
        g_active      = false;
        g_angle_valid = false;
        g_remainder   = 0.0f;
        return false;
    }

    float dx, dy;
    to_centered_001mm(abs_x, abs_y, &dx, &dy);
    const float r = sqrtf(dx * dx + dy * dy);

    if (!g_active) {
        // キーが押されていれば、1 本指ならどこからでも開始
        bool start = (engage && fingers == 1);
#ifdef TP_CIRC_AUTO_RING
        // 外周で触り始めたときも開始
        if (touch_down && r >= RING_001MM) start = true;
#endif
        if (!start) {
            return false;
        }
        // 一度始まったら、指を全部離すまで続ける（キーを離しても続く）
        g_active      = true;
        g_angle_valid = false;
        g_remainder   = 0.0f;
    }

    if (fingers != 1) {
        // 途中で 2 本以上触れたら一時停止。1 本に戻ったら続きから
        g_angle_valid = false;
        return true;
    }

    if (r < DEADZONE_001MM) {
        // 中心付近は角度が不安定。出力せず、次に外へ出たところから測り直す
        g_angle_valid = false;
        return true;
    }

    // 画面座標系（Y が下向き）では、atan2 の増加方向が時計回りになる
    const float angle = atan2f(dy, dx);
    if (!g_angle_valid) {
        g_prev_angle  = angle;
        g_angle_valid = true;
        return true;
    }

    float d = angle - g_prev_angle;
    if (d > (float)M_PI) d -= 2.0f * (float)M_PI;
    if (d < -(float)M_PI) d += 2.0f * (float)M_PI;
    g_prev_angle = angle;

    // 弧の長さ（0.01mm）→ X 解像度の単位。チップの 2 本指スクロールと同じ単位にそろえる
    const float arc = d * r * g_res_x / PAD_W_001MM + g_remainder;
    int32_t     out = (int32_t)arc;
    if (out > INT16_MAX) out = INT16_MAX;
    if (out < INT16_MIN) out = INT16_MIN;
    g_remainder = arc - (float)out;

    *scroll = (int16_t)out;
    return true;
}
