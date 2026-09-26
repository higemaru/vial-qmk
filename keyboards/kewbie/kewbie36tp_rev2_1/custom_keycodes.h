/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

// QMKの既存キーコードと衝突しない安全な範囲から、新しいキーコードを定義します
enum custom_keycodes {
    TP_TOGG_INV = QK_KB_0,  // スクロール方向をトグル（切り替え）
    TP_SPEED_INC,           // スクロール速度を 1 上げる
    TP_SPEED_DEC,           // スクロール速度を 1 下げる
    TP_SPEED_RST,           // スクロール速度をリセット
    TP_TOGG_EN,             // トラックパッド有効/無効 トグル
    TP_DRAG_LOCK,           // ドラッグロック
    TP_INERTIA,             // 慣性スクロール ON/OFF
    TP_CUR_INC,             // カーソル速度を 1 上げる
    TP_CUR_DEC,             // カーソル速度を 1 下げる
    TP_CUR_RST              // カーソル速度をリセット
};

// EEPROM内の設定値の位置（EECONFIG の kb 領域 4 バイト内のバイト位置）
#define EEP_TP_INVERT 0  // 0番地: スクロール方向 (0 or 1)
#define EEP_TP_SPEED  1  // 1番地: スクロール速度 (1-10)
#define EEP_TP_EN     2  // 2番地: トラックパッド有効 (0 or 1)
#define EEP_TP_CUR    3  // 3番地: カーソル速度 (1-10)  ※kb 領域（4バイト）はこれで満杯

#define TP_SPEED_DEFAULT 3  // スクロール速度デフォルト
#define TP_CUR_DEFAULT   5  // カーソル速度デフォルト（等倍）
