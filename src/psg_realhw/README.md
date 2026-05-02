# PC6001VX で PSGだけ実ハードで演奏させる

## 概要

[Raspberry Pi の GPIO に YM2149F をつなぐハードウェア](https://github.com/tsutsui/rpi-psgplayer) を使って、
PC-6001 エミュレータの [PC6001VX](https://github.com/eighttails/PC6001VX) の
PSG演奏エミュレーション部分を差し替えて、ラズパイGPIOに接続された
PSGチップのレジスタを叩いて実ハードで演奏させるための差分です。

## ハードウェアについて

詳細は以下を参照してください

- [rpi-psgplayer の README](https://github.com/tsutsui/rpi-psgplayer/blob/main/README.md)
- 上記で参照しているセミナー資料
- [rpi-psgplayer の回路図説明](https://github.com/tsutsui/rpi-psgplayer/blob/main/hw/SCHEMATIC.md)

## ソース実装

- `src/psg_realhw`
  - 実PSGハード関連で追加したファイルのフォルダ

- `src/psg_realhw/psg_backend_rpi_gpio.c`
  `src/psg_realhw/psg_backend_rpi_gpio.h`
  `src/psg_realhw/psg_backend.h`
  - ラズパイ GPIO接続PSGレジスタアクセス処理と 2MHzクロック出力処理

- `src/psg_realhw/psg_realhw_rpi_gpio.cpp`
  `src/psg_realhw/psg_realhw_backend.h`
  - PC6001VX 実行時のPSGハード用GPIO初期化処理および終了処理と
    レジスタアクセスのラッパ関数
  - 初期化は `std::call_once()` で呼び出し、終了処理は `atexit()` で呼び出し

- `src/device/ay8910.cpp`
  - PC6001VX の PSGレジスタアクセス処理部分に実PSGハードアクセス呼び出しを追加

- `PC6001VX.pro`
  - 実PSGハード演奏関連ファイル定義を追加 (`HAVE_PSG_REALHW` で有効化)

## NetBSD/evbarm でのラズパイでの実行

- 前提として [rpi-psgplayer](hhttps://github.com/tsutsui/rpi-psgplayer/README.md#想定環境)
  と同様に以下の config 設定を追加してビルドしたカーネルが必要です
  - `options         HZ=1000`
  - `options         INSECURE`

- Raspberry Pi 1/2/3 が動作している 32 bit の NetBSD/evbarm 上で
  この `rpi-psg-player` ブランチで PC6001VX をビルド
  - 野良ビルドよりも pkgsrc を展開したあとの `pkgsrc/emulators/PC6001VX` 内で
    `make patch` した後にブランチ差分を手動で当てて
    `make package` して pkgsrc の枠組みでビルドするほうが楽だと思います

- `sudo PC6001VX` で実行
  - この場合 PC6001VX の設定ファイル (テープやフロッピーのデフォルトフォルダを含む)
    は `/root/.pc6001vx4` 以下になるのでデモの場合はそこにファイルを用意してください

- ラズパイ以外の NetBSD で実行した場合は、おそらく GPIO の mmap() で失敗するはず？

- PSGハードを接続していないラズパイで実行した場合、
  特に害は無いはずですが意味もないはずです

## その他

- [rpi-psgplayer](hhttps://github.com/tsutsui/rpi-psgplayer) の実装で
  ハードウェアアクセスレイヤをきっちり分離した恩恵(?)で、
  バックエンド側はほぼそのまま流用する形で実装できました。

- エミュレータ実行でのPSGレジスタアクセスは結構ジッタがあるのではと思っていたのですが、
  実際の PSG演奏ドライバは 2ms割り込み✕6〜10回ごと、つまり
  10〜20ms に1回で 96分音符相当の変化、かつ、プロセスは HZ=1000の 1ms切り替えなので
  耳で聞いて判るほどの遅れは発生しないようです。

- PC6001VX の PSGエミュレーションはそのまま残しているので、実ハードPSGとともに
  エミュレータ本来の PSG演奏もオーディオデバイス側から出力されるはずです。

- PC6001VX 4.3.0 を NetBSD 11.0 もしくは -current で野良ビルドする場合、
  [`isprint` に対するパッチ](https://github.com/NetBSD/pkgsrc/blob/9c4590d6/emulators/PC6001VX/patches/patch-src_console.cpp)
  が必要です

## ライセンス

このフォルダ内のファイルのライセンスは流用元の
[rpi-psgplayer](hhttps://github.com/tsutsui/rpi-psgplayer/LICENSE)
に準じます。
