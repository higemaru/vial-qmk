/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#define VIAL_KEYBOARD_UID {0x53, 0xAC, 0x98, 0x1C, 0xCC, 0x52, 0xFC, 0x19}

/* CONSIDER ADDING AN UNLOCK COMBO. SEE DOCUMENTATION. */
#define VIAL_INSECURE
/* #define VIAL_UNLOCK_COMBO_ROWS { 0, 0 } */
/* #define VIAL_UNLOCK_COMBO_COLS { 0, 11 } */

#define WS2812_DI_PIN GP16

#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1
#define I2C_DRIVER I2CD0

/* #ifdef WS2812_DI_PIN */
/* #	define RGBLED_NUM 1 */
/* #	define RGBLIGHT_LIMIT_VAL 120 */
/* #	define RGBLIGHT_DISABLE_KEYCODES */
/* #	define RGBLIGHT_EFFECT_RAINBOW_MOOD */
/* #	define RGBLIGHT_SLEEP */
/* #	define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_RAINBOW_MOOD */
/* #endif */

#define PIMORONI_TRACKBALL_SCALE 15    // default 5
#define POINTING_DEVICE_ROTATION_90

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
// Time layer remains active after activation
#define AUTO_MOUSE_TIME 450

