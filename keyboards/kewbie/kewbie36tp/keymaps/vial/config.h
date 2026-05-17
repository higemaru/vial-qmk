/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once
#include "config.h"

#define VIAL_KEYBOARD_UID {0x2B, 0xD7, 0xF9, 0xE9, 0x7A, 0xF9, 0x29, 0xB2}

/* CONSIDER ADDING AN UNLOCK COMBO. SEE DOCUMENTATION. */
#define VIAL_INSECURE
/* #define VIAL_UNLOCK_COMBO_ROWS { 0, 0 } */
/* #define VIAL_UNLOCK_COMBO_COLS { 0, 11 } */

#define WS2812_DI_PIN GP16

#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1
#define I2C_DRIVER I2CD0

/* TRACK PAD TPS43-201A-S */
/* https://docs.qmk.fm/features/pointing_device */
#define F_SCL 100000
#define AZOTEQ_IQS5XX_TPS43 1
#define AZOTEQ_IQS5XX_SCROLL_INITIAL_DISTANCE 30 // default 50
#define AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE false // 2本指タップ（右クリック）無効化
/* #define AZOTEQ_IQS5XX_ROTATION_180 */
/* #define AZOTEQ_IQS5XX_SWIPE_X_ENABLE true */

/* #define MOUSE_EXTENDED_REPORT */ // is_keyboard_master_os_windows を使って、コンパイル時に OS 判別

/* #ifdef WS2812_DI_PIN */
/* #	define RGBLED_NUM 1 */
/* #	define RGBLIGHT_LIMIT_VAL 120 */
/* #	define RGBLIGHT_DISABLE_KEYCODES */
/* #	define RGBLIGHT_EFFECT_RAINBOW_MOOD */
/* #	define RGBLIGHT_SLEEP */
/* #	define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_RAINBOW_MOOD */
/* #endif */

/* TRACK BALL AZ1UBALL */
/*
#define PIMORONI_TRACKBALL_SCALE 10    // default 5
#define POINTING_DEVICE_ROTATION_90
*/
