# CLOADのリソース

[日本語一覧](../../README.md) | [抽出ツール](../../../../tools/data/overlays/README.md) | [English](../../../en/technical/reference/cload-resources.md)

CLOADのメッセージと小さなテーブルは、1つのデータblobに入っています。
ビルドも両バージョンで、すでにそのままリンクしています。中身を見るには、
まずsplatで素材を抽出してから、次のコマンドを実行します。

```sh
make splat
make extract-cload

make splat VERSION=jp
make extract-cload VERSION=jp
```

出力先は`assets/exports/<version>/overlays/cload/`です。
`CLOAD_OUTPUT=/path/to/new-folder`で別の場所を指定できます。
既存のフォルダは上書きしません。出力は中身を確認するためのもので、
ビルドは引き続き元のblobを読みます。

## blobに入っているもの

| 出力 | 内容 |
| --- | --- |
| `text/messages.yaml` | 北米版91件、日本版90件のメッセージ。番号、判明しているシンボル名、元のバイト列付き |
| `text/locations.yaml` | セーブに記録された曲番号で選ぶ、63件の場所名エントリ |
| `tables/card_steps.yaml` | 6つのカード処理シーケンス。処理名は`CloadLoadStep`から取得 |
| `tables/text_conversion.yaml` | ゲームの文字コードとShift-JISの対応表 |
| `tables/digit_glyphs.yaml` | 10進数と16進数に使う全角数字 |
| `byte-map.yaml` | パディングと実行時バッファも含む、全範囲の対応表 |

メッセージと場所名は、ADDHEROやCARDAにあるテーブルのコピーです。
そのため、CLOADでは使わないメッセージも出力に含まれます。
北米版の辞書コードや制御コードは波括弧で表示し、日本版はCLOAD自身の文字表で
デコードします。どちらもテキストの横に元のバイト列を残します。

末尾の149,920バイトはCLOADの変数とバッファで、ディスク上ではすべてゼロです。
この範囲はバイトマップに記録し、別のファイルにはしません。
リソース間の隙間も両バージョンですべてゼロなので、今回の出力には不明データは
ありません。未解釈の範囲にゼロ以外のバイトがあれば、`unknown/`にそのまま保存します。

## アイコンはどこにある？

CLOADは[`cload_load_icon_resources`](../../../../src/overlays/cload/cload_widgets.c)で、
CDリソース`0x5E4`からパーティーアイコンを読み込みます。
このblobには含まれていないため、今回の抽出ではPNGは出力しません。
メモリーカードのタイトル雛形もblobにはありません。固定のカードパス文字列は
[`cload_card.c`](../../../../src/overlays/cload/cload_card.c)で定義されています。

[`cload.py`](../../../../tools/data/overlays/cload.py)はアドレス順にblobを読み、
共通のカード形式の読み取り処理を使います。アドレスは各バージョンのシンボルファイル、
処理名はCのenumから取得します。2バイト文字表の基準アドレスが
[`cload_glyph.c`](../../../../src/overlays/cload/cload_glyph.c)と一致することもテストで確認します。
