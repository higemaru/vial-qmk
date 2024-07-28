/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#define VIAL_KEYBOARD_UID {0xB0, 0x65, 0x92, 0x1D, 0x53, 0x40, 0xFF, 0xFF}

/* CONSIDER ADDING AN UNLOCK COMBO. SEE DOCUMENTATION. */
#define VIAL_INSECURE
/* #define VIAL_UNLOCK_COMBO_ROWS { 0, 0 } */
/* #define VIAL_UNLOCK_COMBO_COLS { 0, 11 } */

#define WS2812_DI_PIN GP16
/* #ifdef WS2812_DI_PIN */
/* #	define RGBLED_NUM 1 */
/* #	define RGBLIGHT_LIMIT_VAL 120 */
/* #	define RGBLIGHT_DISABLE_KEYCODES */
/* #	define RGBLIGHT_EFFECT_RAINBOW_MOOD */
/* #	define RGBLIGHT_SLEEP */
/* #	define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_RAINBOW_MOOD */
/* #endif */
