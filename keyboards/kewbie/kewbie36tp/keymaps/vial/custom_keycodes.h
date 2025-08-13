#pragma once

// QMKの既存キーコードと衝突しない安全な範囲から、新しいキーコードを定義します
enum custom_keycodes {
    TP_TOGG_INV = SAFE_RANGE, // スクロール方向をトグル（切り替え）
    TP_SPEED_INC,             // スクロール速度を 1 上げる
    TP_SPEED_DEC,             // スクロール速度を 1 下げる
    TP_SPEED_RST              // スクロール速度をリセット
};

// EEPROM内の設定値保存場所を定義します（ユーザー領域の先頭からのオフセット）
#define EEP_TP_INVERT 0 // 0番地: スクロール方向 (0 or 1)
#define EEP_TP_SPEED  1 // 1番地: スクロール速度 (1-10)
