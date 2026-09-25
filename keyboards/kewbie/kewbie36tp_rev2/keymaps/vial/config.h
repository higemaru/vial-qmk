/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* Vial 固有の設定のみ。
 * ハードウェア定義（I2C・TPS43・Azoteq設定）は
  * keyboards/kewbie36tp/config.h にある。
 */
// python3 util/vial_generate_keyboard_uid.py
#define VIAL_KEYBOARD_UID {0x4C, 0x56, 0xD7, 0xFB, 0x05, 0xD7, 0x23, 0x51}

/* CONSIDER ADDING AN UNLOCK COMBO. SEE DOCUMENTATION. */
#define VIAL_INSECURE
/* #define VIAL_UNLOCK_COMBO_ROWS { 0, 0 } */
/* #define VIAL_UNLOCK_COMBO_COLS { 0, 11 } */
