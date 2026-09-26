/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* ============================================================
 * 円周スクロール（ハード非依存のロジック部）
 *
 * トラックパッドの絶対座標から「外周で触り始めて円を描いたか」を判定し、
 * 円周に沿った移動量（弧の長さ）をスクロール量として返す。
 *
 * - 判定は触り始めた瞬間の1回だけ（0本 → 1本 になったとき）
 * - 外周で触り始めたら、指を離すか本数が変わるまで円周スクロール
 * - 内側で触り始めたら何もしない（通常のカーソル移動に任せる）
 *
 * QMK に依存しないので、ホストの gcc で単体テストできる。
 * ============================================================ */

#include <stdbool.h>
#include <stdint.h>

/* ---- パッドの物理サイズ（TPS43 = 43 x 40 mm） ----
 * AZOTEQ_IQS5XX_ROTATION_90/270 で XY を入れ替える場合は、ここも入れ替えること。 */
#ifndef TP_PAD_WIDTH_MM
#    define TP_PAD_WIDTH_MM 43
#endif
#ifndef TP_PAD_HEIGHT_MM
#    define TP_PAD_HEIGHT_MM 40
#endif

/* ---- カバーの円形切り抜き ----
 * 直径と、パッド中心からのずれ（0.1mm 単位、右・下が正）。 */
#ifndef TP_CIRC_DIAMETER_MM
#    define TP_CIRC_DIAMETER_MM 38
#endif
#ifndef TP_CIRC_CENTER_OFFSET_X_01MM
#    define TP_CIRC_CENTER_OFFSET_X_01MM 0
#endif
#ifndef TP_CIRC_CENTER_OFFSET_Y_01MM
#    define TP_CIRC_CENTER_OFFSET_Y_01MM 0
#endif

/* ---- 外周リングの幅 ----
 * 半径の何 % より外で触り始めたら円周スクロールにするか。
 * 70 なら外側 30% がリング。 */
#ifndef TP_CIRC_RING_PCT
#    define TP_CIRC_RING_PCT 70
#endif

/* ---- 中心付近の無視 ----
 * 円周スクロール中に指が中心近くまで入ると角度が暴れるので、
 * 半径の何 % より内側では移動量を出さない。 */
#ifndef TP_CIRC_DEADZONE_PCT
#    define TP_CIRC_DEADZONE_PCT 25
#endif

/* 解像度（絶対座標の最大値）を設定する。CPI が変わったら呼び直す。 */
void tp_circ_set_resolution(uint16_t res_x, uint16_t res_y);

/* 毎フレーム呼ぶ。
 *   fingers : 指の本数
 *   abs_x/y : 1本目の指の絶対座標
 *   scroll  : 出力。弧の長さ（パッドの X 解像度と同じ単位）。時計回りが正
 * 戻り値   : 円周スクロール中なら true（呼び出し側はカーソル移動を止める） */
bool tp_circ_update(uint8_t fingers, uint16_t abs_x, uint16_t abs_y, int16_t *scroll);

/* 円周スクロール中かどうか */
bool tp_circ_is_active(void);
