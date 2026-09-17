/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* ============================================================
 * ハードウェア定義
 *
 * 基板の配線そのものを表す。キーマップに依存しないものはすべてここ。
 * keymaps 配下の config.h には Vial 固有の設定だけを置く。
 *
 * 本番基板(RP2040直載せ、マトリクス左右分割済み)向け。
 * WS2812は未実装のため定義しない。
 * ============================================================ */

/* ---- I2C（TPS43 トラックパッド） ----
 * GP18 = I2C1 SDA、GP19 = I2C1 SCL なので I2CD1。
 * RP2040 は GPn の n%4 で決まる：0=I2C0 SDA / 1=I2C0 SCL / 2=I2C1 SDA / 3=I2C1 SCL
 *
 * 定義名の "I2C1_" はペリフェラル番号ではない。QMK は I2C を 1 個しか
 * サポートしないため、どのドライバを選んでも I2C1_* が使われる。
 * mcuconf.h 側は RP_I2C_USE_I2C1 TRUE にしてある。 */
#define I2C_DRIVER   I2CD1
#define I2C1_SDA_PIN GP18
#define I2C1_SCL_PIN GP19
#define F_SCL        100000

/* ---- TPS43 の制御線 ---- */
#define TPS43_RST_PIN GP20   // FPC1 pin2 / NRST
#define TPS43_RDY_PIN GP21   // FPC1 pin1 / RDY（現時点ではゲートに未使用。詳細は kewbie36tp.c）

/* ---- TPS43-201A-S（Azoteq IQS572） ----
 * https://docs.qmk.fm/features/pointing_device
 * 搭載チップは実機検証で IQS572（Product Number 58）と確定済み。 */
#define AZOTEQ_IQS5XX_TPS43 1
#define AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE 30    // default 50
#define AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE false   // 2本指タップ（右クリック）無効化
/* #define AZOTEQ_IQS5XX_ROTATION_180 */
/* #define AZOTEQ_IQS5XX_SWIPE_X_ENABLE true */

/* #define MOUSE_EXTENDED_REPORT */
