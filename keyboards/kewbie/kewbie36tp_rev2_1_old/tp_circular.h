/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* ============================================================
 * 円周スクロール（ハード非依存のロジック部）
 *
 * 1 本指の絶対座標から、パッド中心の周りを回った量（弧の長さ）を
 * スクロール量として返す。
 *
 * 開始のきっかけは 2 つ。
 *   1. engage（呼び出し側が決める。キーを押している間など）が true のとき、
 *      指が 1 本触れていれば、どこからでも開始
 *   2. TP_CIRC_AUTO_RING を定義した場合は、外周で触り始めたときも開始
 *
 * 一度始まったら、指を全部離すまで続く（engage が false に戻っても続く）。
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

/* ---- 回転の中心と半径 ----
 * 直径と、パッド中心からのずれ（0.1mm 単位、右・下が正）。
 * 半径はリング判定と中心付近の無視範囲の基準に使う。 */
#ifndef TP_CIRC_DIAMETER_MM
#    define TP_CIRC_DIAMETER_MM 38
#endif
#ifndef TP_CIRC_CENTER_OFFSET_X_01MM
#    define TP_CIRC_CENTER_OFFSET_X_01MM 0
#endif
#ifndef TP_CIRC_CENTER_OFFSET_Y_01MM
#    define TP_CIRC_CENTER_OFFSET_Y_01MM 0
#endif

/* ---- 外周から触り始めたら自動で開始（任意） ----
 * 定義すると、キーを押していなくても、半径の TP_CIRC_RING_PCT % より外で
 * 触り始めたら円周スクロールになる。 */
/* #define TP_CIRC_AUTO_RING */
#ifndef TP_CIRC_RING_PCT
#    define TP_CIRC_RING_PCT 70
#endif

/* ---- 中心付近の無視 ----
 * 中心近くでは角度が暴れるので、半径の何 % より内側では移動量を出さない。 */
#ifndef TP_CIRC_DEADZONE_PCT
#    define TP_CIRC_DEADZONE_PCT 25
#endif

/* 解像度（絶対座標の最大値）を設定する。CPI が変わったら呼び直す。 */
void tp_circ_set_resolution(uint16_t res_x, uint16_t res_y);

/* 毎フレーム呼ぶ。
 *   fingers : 指の本数
 *   abs_x/y : 1 本目の指の絶対座標
 *   engage  : 円周スクロールを始めてよいか（キーを押している間など）
 *   scroll  : 出力。弧の長さ（パッドの X 解像度と同じ単位）。時計回りが正
 * 戻り値   : 円周スクロール中なら true（呼び出し側はカーソル移動を止める） */
bool tp_circ_update(uint8_t fingers, uint16_t abs_x, uint16_t abs_y, bool engage, int16_t *scroll);

/* 円周スクロールを始めてよいか。キーボード側で実装する（ドライバが毎フレーム呼ぶ）。
 * 定義しなければ常に false（TP_CIRC_AUTO_RING だけで動く）。 */
bool tp_circ_engaged(void);

/* 円周スクロール中かどうか */
bool tp_circ_is_active(void);
