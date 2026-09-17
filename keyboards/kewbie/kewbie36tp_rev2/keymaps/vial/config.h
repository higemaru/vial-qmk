/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* Vial 固有の設定のみ。
 * ハードウェア定義（I2C・TPS43・Azoteq設定）は
 * keyboards/kewbie36tp/config.h にある。 */

#define VIAL_KEYBOARD_UID {0x2B, 0xD7, 0xF9, 0xE9, 0x7A, 0xF9, 0x29, 0xB2}

/* CONSIDER ADDING AN UNLOCK COMBO. SEE DOCUMENTATION. */
#define VIAL_INSECURE
/* #define VIAL_UNLOCK_COMBO_ROWS { 0, 0 } */
/* #define VIAL_UNLOCK_COMBO_COLS { 0, 11 } */
