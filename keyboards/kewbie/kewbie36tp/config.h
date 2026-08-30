/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* ============================================================
 * ハードウェア定義
 *
 * 基板の配線そのものを表す。キーマップに依存しないものはすべてここ。
 * keymaps 配下の config.h には Vial 固有の設定だけを置く。
 * ============================================================ */

/* ---- WS2812（RP2040-Zero 内蔵 LED） ----
 * keyboard.json の rgblight.pin でも指定済み。明示のため残している。 */
#define WS2812_DI_PIN GP16

/* ---- I2C（TPS43 トラックパッド） ----
 * GP0 = I2C0 SDA、GP1 = I2C0 SCL なので I2CD0。
 * RP2040 は GPn の n%4 で決まる：0=I2C0 SDA / 1=I2C0 SCL / 2=I2C1 SDA / 3=I2C1 SCL
 *
 * 定義名の "I2C1_" はペリフェラル番号ではない。QMK は I2C を 1 個しか
 * サポートしないため、どのドライバを選んでも I2C1_* が使われる。 */
#define I2C_DRIVER   I2CD0
#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1
#define F_SCL        100000

/* ---- TPS43-201A-S（Azoteq IQS572） ----
 * https://docs.qmk.fm/features/pointing_device */
#define AZOTEQ_IQS5XX_TPS43 1
#define AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE 30    // default 50
#define AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE false   // 2本指タップ（右クリック）無効化
/* #define AZOTEQ_IQS5XX_ROTATION_180 */
/* #define AZOTEQ_IQS5XX_SWIPE_X_ENABLE true */

/* #define MOUSE_EXTENDED_REPORT */
