# kewbie36tp — kewbie36tp.c への整理

現行基板（RP2040-Zero / GP0・GP1）向け。**動作を変えないリファクタのみ。**
ピン割り当て・マトリクス・慣性スクロールのパラメータは一切触っていない。

## 変更後の構成

```
keyboards/kewbie36tp/
├── config.h              ★新規  ハードウェア定義（I2C・WS2812・Azoteq）
├── custom_keycodes.h     ★移動  keymaps/vial/ から昇格
├── kewbie36tp.c          ★新規  トラックパッド処理いっさい
├── keyboard.json                 変更なし
├── halconf.h / mcuconf.h         変更なし
├── rules.mk                      変更なし
└── keymaps/
    ├── default/keymap.c          keymaps 配列のみに
    │   └── config.h      ★削除  config.h に統合
    ├── via/keymap.c              keymaps 配列のみに
    └── vial/
        ├── config.h              Vial 固有の設定のみに
        ├── keymap.c              keymaps 配列のみに
        ├── rules.mk              変更なし
        └── vial.json             変更なし
```

## 関数の移動

| 移動前（keymaps/vial/keymap.c） | 移動後（kewbie36tp.c） |
|---|---|
| `keyboard_post_init_user()` | `keyboard_post_init_kb()` |
| `eeconfig_init_user()` | `eeconfig_init_kb()` |
| `process_record_user()` | `process_record_kb()` |
| `pointing_device_task_user()` | `pointing_device_task_kb()` |
| `tp_cache_load()` ほか静的変数 | そのまま移動 |

すべて末尾で `_user` 版を呼んでいる。呼ばないと keymap 側で同名関数を
定義しても動かなくなる。

## なぜ _kb 層が正しいか

Vial の `customKeycodes` は `QK_KB_0` 起点、つまり**キーボードレベル**の
キーコード。`TP_TOGG_EN` などを `_user` で拾うのは層がずれている。

トラックパッドは基板の配線そのもので、キーマップを差し替えても存在する。
`_kb` に置けば default / via / vial のどれをビルドしても同じ挙動になる。

## 修正した点

### 1. ハードウェア定義の重複

`keymaps/default/config.h` と `keymaps/vial/config.h` に I2C ピンと
WS2812 ピンが**二重に書かれていた**。`keyboards/kewbie36tp/config.h` に
一本化し、default 側は削除した。

### 2. `#include "config.h"` が自己インクルードだった

`keymaps/vial/config.h` の冒頭にあったこの行は、同ディレクトリの自分自身を
指していて `#pragma once` で弾かれる**完全な no-op** だった。削除した。

キーボードレベルの `config.h` は QMK が自動でインクルードするので、
keymap 側から明示的に読む必要はない。

### 3. `custom_keycodes.h` の閉じ括弧が壊れていた

```c
    TP_INERTIA                // 慣性スクロール ON/OFF};
};
```

`};` がコメント内に入り込んでいた。次行の `};` が効いていたので
コンパイルは通っていたが、意図と違う。整形した。

### 4. default / via の旧スクロール処理を削除

両者に mac 向け固定倍率の `pointing_device_task_user` が残っていた。
`_kb` で処理するようになると**二重適用**になるため削除した。

スクロール方向は `_kb` の EEPROM 設定に従うようになる。

### 5. デフォルトのスクロール方向を Mac に変更

```c
uint8_t default_invert = 1;              // ← Mac（ナチュラル）
#ifdef OS_DETECTION_ENABLE
    if (detected_host_os() == OS_WINDOWS) {
        default_invert = 0;              // Windows と判定できたときだけ従来方向
    }
#endif
```

条件を反転させ、フォールバック値を 1 にした。OS 判別が有効な場合の
挙動（Windows → 0、それ以外 → 1）は従来と同じ。

> **OS 判別はあてにしないこと。**
> `eeconfig_init_kb()` は EEPROM が無効なとき（初回書き込み／`EE_CLR` 直後）に
> 呼ばれるが、このタイミングでは USB の列挙が完了しておらず
> `detected_host_os()` は `OS_UNSURE` を返す可能性が高い。
>
> つまり実質的に効いているのは**フォールバック値**のほう。
> 確実に固定したいなら `#ifdef` ブロックごと消して
> `uint8_t default_invert = 1;` だけにしてもよい。
>
> 変更後に反映させるには **`EE_CLR` が必要**（保存済みの値が優先されるため）。

## そのままにした点

- **`EECONFIG_USER` のアドレス** — 層としては `EECONFIG_KB` が正しいが、
  変えると保存済みの設定が飛ぶ。据え置きが安全
- **`TP_INERTIA` が未割り当て** — キーコードは定義済みだが keymap のどこにも
  置かれていない。使うなら layer 3 の空きに割り当てる
- **`vial.json`** — マトリクスもレイアウトも変えていないので変更不要

## 書き込み後の確認

EEPROM のレイアウトは変えていないので、リファクタ自体に `EE_CLR` は不要。
ただし**デフォルトのスクロール方向の変更を反映させるには `EE_CLR` が要る**
（保存済みの値が優先されるため）。既に Mac 方向に設定済みなら不要。

1. ビルドが通る
2. キー入力が従来どおり
3. スクロールの向き・速度が保存値のまま
4. 慣性が効く
5. `TP_TOGG_EN` / `TP_SPEED_*` / `TP_DRAG_LOCK` が反応する

5 が動かない場合、`process_record_kb` が `process_record_user` を
呼べていないか、`custom_keycodes.h` のインクルードパスを疑う。

## 次のステップ

新基板へ移行するときは `kewbie36tp.c` 末尾のコメントブロックを有効化する。

- `keyboard_pre_init_kb()` で TPS43 の RST（GP20）を High にする
- `config.h` の I2C を `I2CD1` / GP18 / GP19 に変更
- `mcuconf.h` を `RP_I2C_USE_I2C1 TRUE` に変更

RDY（GP21）を使わない理由も同ファイルにコメントで残してある。
