# Legend of Mana PSX Decompilation

**言語:** [English](README.md) | 日本語

[![Progress]][progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[Progress]: https://decomp.dev/celophi/lom-decomp.svg?mode=shield&measure=code&category=all&label=Progress
[progress site]: https://decomp.dev/celophi/lom-decomp

PlayStation用ソフト **『聖剣伝説 LEGEND OF MANA』** を、オリジナルと一致する形で復元する**デコンパイルプロジェクト**です。北米版は **100%一致**しており、日本版は現在対応を進めています。

本プロジェクトでは、ディスク上のオリジナルのMIPS機械語とバイト単位で一致するように、読みやすいCソースコードを復元しています。対象としているリージョンは次の2つです。

- **北米版** - `SLUS_010.13`（ディスクシリアル **SLUS-01013**）。完了済みで、18個すべてのバイナリが完全にリンクされています。
- **日本版** - `SLPS_021.70`（ディスクシリアル **SLPS-02170**）。対応中で、まだビルドには組み込まれていません。

特に記載がない限り、以下のビルド手順、ターゲット、ファイル名は北米版を対象としています。

これはデコンパイルプロジェクトであり、**PC移植版ではありません**。このリポジトリには、ゲームの実行ファイル、オーバーレイバイナリ、アートワーク、音声、その他の著作権で保護されたゲームデータは含まれていません。必要なファイルは、各自が所有するゲームから用意してください。

このプロジェクトの主な目的は、教育・研究や将来的なMod制作の可能性に向けて、オリジナルゲームのロジックと挙動を保存することです。

## 完全リンク

北米版では、メイン実行ファイルと17個すべてのオーバーレイを含む全モジュールが**完全リンク済み**です。本プロジェクトでは、次の2つの条件を満たしたモジュールを完全リンク済みとしています。

1. ビルドによって生成された**ELFのバイト列が、オリジナルを展開したファイルと一致する**こと。
2. そのELFをrawバイナリに変換し、本プロジェクトのコンプレッサーで圧縮すると、**ディスク上の元の `.BIN` ファイルと完全に同一のファイルを再生成できる**こと。

つまり、`original .BIN -> decompress -> C source -> compile -> ELF -> compress -> .BIN` という一連の往復処理がビット単位で完全一致します。

メイン実行ファイルは圧縮されていないため、`SLUS_010.13` では1つ目の条件だけが確認対象です。リンク済みELFをrawバイナリへ変換したものが、ディスク上のファイルと一致します。

**（コンプレッサーもぜひ見てみてください。正直ここまでやる必要はないのですが、元のファイルと _ビット単位で完全一致_ するのはかなり面白いです！）**

すべてのモジュールを確認するには `make verify-bins` を実行してください。日本版はまだこの段階には到達していません。

## ロードマップ

1. ✅ **100%一致** - **完了。** メイン実行ファイル（`SLUS_010.13`）と17個すべてのオーバーレイが完全リンク済みです（上記参照）。Psy-Q SDKライブラリについては、引き続きオリジナルのアセンブリをリンクしています。

2. 🚧 **クリーンアップとドキュメント整備** - **進行中。** デコンパイル作業中に生じた不自然なコードを整理し、機能をドキュメント化します。

3. 🚧 **NTSC-J版** - **進行中。** 北米版と並行して、日本版（`SLPS-02170`）にも対応します。

4. 💤 **Mod制作とソースポート** - 復元したソースコードを基盤として、実用的なMod制作や他プラットフォームへの移植を可能にします。

## 対応ゲームバージョン

| 項目 | 北米版 | 日本版 |
|---|---|---|
| 状態 | ✅ 完全リンク済み | 🚧 対応中 |
| ディスクシリアル | `SLUS-01013` | `SLPS-02170` |
| メイン実行ファイル | `SLUS_010.13` | `SLPS_021.70` |
| メイン実行ファイル SHA-1 | `d11dfdd50d412ac3fa3e2eb80fbde138da118f27` | `b067188a92e4de9a4db7bb7e5343c757e9884bfa` |
| ディスクイメージ（`.bin`）SHA-1 | `c1b536c99f0d390584eb30462a7e37f2bbef3902` | `7a314615be8a482cf3f81b4101cc19aaa738f36d` |
| アーキテクチャ | 32-bit little-endian MIPS / PlayStation | 32-bit little-endian MIPS / PlayStation |

日本版はまだビルド設定に組み込まれていないため、以下の導入手順は北米版のみを対象としています。その他のリージョンには現在対応していません。

## 必要なもの

通常のビルドには次のものが必要です。

- **Git** - サブモジュールを利用できること。
- **Docker** - Windows/macOSではDocker Desktop、LinuxではDocker Engine。
- **正規に入手した北米版『Legend of Mana』**。日本版はまだビルドできません（[対応ゲームバージョン](#対応ゲームバージョン)を参照）。

当時のPSXコンパイラ、Psy-Qツール、Pythonパッケージ、MIPSクロスコンパイラなどをホスト環境へ直接インストールする必要はありません。開発用コンテナに必要なものが含まれています。

## はじめに

### 1. リポジトリをクローンする

```bash
git clone --recursive https://github.com/celophi/lom-decomp.git
cd lom-decomp
```

サブモジュールなしで既にクローンしている場合は、次を実行してください。

```bash
git submodule update --init --recursive
```

### 2. オリジナルのゲームファイルを配置する

現在のビルドには北米版が必要です（日本版にも近日対応予定です）。北米版のディスクまたはディスクイメージから、メイン実行ファイルとゲームの `BIN` ディレクトリを抽出し、リポジトリ内を次の構成にしてください。

```text
disc/
|-- SLPS_021.70
`-- BIN/
    |-- ADDHERO.BIN
    |-- CARDA.BIN
    |-- CHECKPS.BIN
    |-- CLOAD.BIN
    |-- FIELD.BIN
    |-- GNAME.BIN
    |-- GOLEM.BIN
    |-- GOSUB.BIN
    |-- GOVER.BIN
    |-- MENU.BIN
    |-- MOVIE.BIN
    |-- NIKI.BIN
    |-- SHOP.BIN
    |-- TITLE.BIN
    |-- WMAP.BIN
    |-- WSEL.BIN
    `-- ZUKAN.BIN
```

ディスクの残りのファイルをリポジトリへコピーする必要はありません。

メイン実行ファイルが想定しているバージョンか確認するには、次を実行します。

```bash
sha1sum disc/SLPS_021.70
```

期待される結果:

```text
b067188a92e4de9a4db7bb7e5343c757e9884bfa  disc/SLPS_021.70
```

（日本版はまだビルドに対応していないため、現時点ではお手元のダンプが正しいかの確認用です。）

splatの設定ファイルには、各オーバーレイファイルの期待されるSHA-1ハッシュも記載されています。

> `disc/` はgitignoreの対象です。オリジナルのゲームファイルは絶対にコミットしないでください。

### 3. 当時のコンパイラ用イメージをビルドする

本プロジェクトでは、複数の古いGCCを使い分けています。`old-gcc` サブモジュールを使って、次の4つのローカルコンパイライメージをビルドしてください。

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/old-gcc/gcc-2.8.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.6.0-psx -f tools/old-gcc/gcc-2.6.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-psx-gnu -f dockerfiles/gnu-as.dockerfile tools/old-gcc
```

通常、この作業が必要なのは最初の1回だけです。開発用Dockerfileはこれらのローカルイメージを利用するため、非公開のコンパイライメージへアクセスする必要はありません。

### 4. 開発用コンテナをビルドする

```bash
docker build -t lom-dev -f dockerfiles/dev.dockerfile .
```

このイメージには、本プロジェクトで使用するコンパイラ、Psy-Qツール、MIPS binutils、splat、maspsx、objdiff関連ツール、Python依存パッケージが含まれています。

### 5. コンテナを起動する

リポジトリのルートディレクトリから実行してください。

PowerShell、bash、zshの場合:

```bash
docker run --rm -it -v "${PWD}:/lom" lom-dev
```

Windows Command Prompt（`cmd.exe`）の場合:

```bat
docker run --rm -it -v "%cd%:/lom" lom-dev
```

これ以降のセットアップコマンドは、**コンテナ内で実行します**。

### 6. オリジナルのバイナリを分割する

```bash
make splat
```

これにより、`asm/`、`linker/`、抽出されたアセットなど、ローカルで使用するビルド入力が生成されます。これらのファイルは意図的にすべてをGitへ保存しているわけではありません。

splat設定、セグメント境界、シンボルマップ、relocation overrideを変更した場合は、`make splat` を再実行してください。

### 7. ビルドする

メイン実行ファイルをビルドします。

```bash
make
```

出力:

```text
build/SLPS_021.70.elf
```

フラットバイナリも生成する場合:

```bash
make bin
```

登録済みのオーバーレイを1つビルドする場合:

```bash
make field
make menu
make checkps
```

`mk/overlay-registry.mk` に現在登録されているすべてのオーバーレイをビルドする場合:

```bash
make overlays
```

メイン実行ファイルと登録済みの全オーバーレイをビルドする場合:

```bash
make everything
```

`mk/overlay-registry.mk` が、現在リンク可能なビルドへ組み込まれているオーバーレイの正式な一覧です。

## 通常の開発フロー

初期セットアップ後は、通常、編集するたびにプロジェクトを**クリーンする必要はありません**。

```text
ホスト側でソースを編集
        |
        v
lom-dev 内で make <必要最小限のターゲット>
        |
        v
objdiff / diff の結果を確認
        |
        v
編集して繰り返す
```

Makefileは、コンパイル前に変更された入力を自動的にステージングします。ステージングされたコピーが古いように見える場合は、次を実行してください。

```bash
make recopy
```

`make clean` は、`build/` と `/staging` のコピーを実際に削除したい場合にのみ使用してください。

## `/staging` が存在する理由

リポジトリはDocker内の `/lom` にマウントされますが、当時のコンパイラによる実際のコンパイルは、コンテナ内のネイティブLinuxファイルシステム上にある `/staging` から行われます。

一部の古い32-bitコンパイラ / プリプロセッサは、Windows側のDocker bind mount上にあるファイルへ安全に `stat()` できず、次のエラーで失敗する場合があります。

```text
Value too large for defined data type
```

Makefileでは、必要な入力を `/staging` へコピーし、テキストファイルの改行コードをLFに正規化してからコンパイルすることで、この問題を回避しています。

そのため、ビルドシステムを迂回して `/lom` 以下のファイルに対して古いコンパイラを直接実行しないでください。

## 便利なMakeターゲット

| ターゲット | 用途 |
|---|---|
| `make` | メインの `SLPS_021.70` ELFをビルドします。 |
| `make bin` | `build/SLPS_021.70.bin` も生成します。 |
| `make <overlay>` | `make field` など、登録済みのオーバーレイを1つビルドします。 |
| `make overlays` | 登録済みの全オーバーレイをビルドします。 |
| `make everything` | メイン実行ファイルと登録済みの全オーバーレイをビルドします。 |
| `make splat` | メイン実行ファイルと全オーバーレイ設定を分割します。 |
| `make objdiff-objects` | objdiff用にターゲット側と復元側のオブジェクトをビルドします。 |
| `make objdiff-config` | `objdiff.json` を再生成します。 |
| `make progress` | `build/progress.json` を生成します。 |
| `make diff-all` | 設定済みの全ユニットに対してobjdiffを実行します。 |
| `make diff-text` | `build/diffs/` 以下に簡潔なテキストレポートを生成します。 |
| `make dump-objs` | コード生成の解析用にビルド済みオブジェクトを逆アセンブルします。 |
| `make validate-assets` | フォーマットを認識するアセットについて、往復変換と検証を行います。 |
| `make verify-slus` | リンク済みのメイン実行ファイルが `disc/SLPS_021.70` と一致するか確認します。 |
| `make verify-bins` | `verify-slus` と、登録済み全オーバーレイのSHA-1チェックを実行します。 |
| `make verify-compressor` | オリジナルの17個すべてのオーバーレイファイルに対してコンプレッサーを検証します。 |
| `make recopy` | ソース / 設定ファイルを `/staging` へ強制的に再コピーします。 |
| `make clean` | ビルド出力と `/staging` を削除します。 |

## 関数のマッチング

本プロジェクトでは、ローカルで関数やオブジェクトを比較するために [objdiff](https://github.com/encounter/objdiff) を使用しています。

比較する両方のオブジェクトをビルドし、objdiffの設定を生成します。

```bash
make objdiff-objects
make objdiff-config
```

その後objdiffを起動し、リポジトリのルートディレクトリを指定してください。

コマンドラインで作業する場合:

```bash
make diff-all
make diff-text
```

簡潔なレポートは `build/diffs/` 以下に出力されます。

共同でマッチング作業を行う場合は [decomp.me](https://decomp.me) も利用できます。既存のソースコメントには多数のdecomp.me scratchへのリンクが含まれているため、関数を編集する際はそれらの参照を残してください。

## コンパイラとアセンブラのツールチェーン

このプロジェクトで特に重要なのは、**すべてのソースファイルが同じコンパイラ設定でビルドされているわけではない**という点です。

ちなみに、この複数のツールチェーンへの対応にはかなり「楽しませてもらいました」。特にGNU版はなかなか強敵でした。

ビルドでは、4種類の古いコンパイラビルドを使い、合計7種類のパイプラインを定義しています。コンパイラとアセンブラのフラグは [`mk/toolchains.mk`](mk/toolchains.mk) にあります。ソースの振り分けとファイル単位のオーバーライドは [`mk/main.mk`](mk/main.mk) と [`mk/overlay-registry.mk`](mk/overlay-registry.mk) にあり、[`mk/overlays.mk`](mk/overlays.mk) がオーバーレイ用の各バリアントを適用します。

| パイプライン | コンパイラフラグ | アセンブル経路 |
|---|---|---|
| GCC 2.8.0 G0（デフォルト） | `-O2 -G0 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G0、最適化なし | `-O0 -G0 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G4、division展開なし | `-O2 -G4 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、divisionをそのまま使用 |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx、ASPSX 2.67、division展開あり |
| GCC 2.7.2 GNU G0 | `-O2 -G0` | 当時のGNU `as`（`-O -EL`） |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx、ASPSX 2.34、division展開あり |

## 著作権で保護されたデータとアセット

実行ファイルやオーバーレイの一部領域には、アートワーク、テキスト、レイアウト、その他の著作権で保護されたデータが含まれているため、そのままコミットすべきではありません。

本プロジェクトでは、次のようなハイブリッド方式を採用しています。

- 内容を理解できたプログラムデータは、型付きのCデータとして表現できます。
- 内容を理解できたバイナリ形式は、バイト単位で正確なextractor / builderを使用できます。
- 未解析のデータや創作性のあるデータは、名前を付けたローカルの `databin` / `rodatabin` アセットとして残し、`.incbin` で参照できます。

詳細:

- [`docs/handling-copyrighted-data.md`](docs/handling-copyrighted-data.md)
- [`docs/asset-data-architecture.md`](docs/asset-data-architecture.md)

## リポジトリ構成

```text
lom-decomp/
|-- src/                    # 復元したCソース
|   |-- overlays/           # オーバーレイのソースツリー
|   `-- psyq/               # 復元したPsy-Qライブラリコード
|-- include/                # プロジェクトおよびPsy-Qのヘッダ / マクロ
|-- config/                 # Splat設定、シンボル、relocation
|-- mk/                     # ビルドルールとツールチェーンの振り分け
|-- tools/                  # デコンパイル、コンパイラ、diff、アセット用ツール
|-- docs/                   # アーキテクチャとマッチング関連ドキュメント
|-- disc/                   # 手元のオリジナルゲームファイル（gitignore対象）
|-- assets/                 # 必要に応じて使用するローカル / 生成アセットデータ
|-- asm/                    # Splatが生成するターゲットアセンブリ（gitignore対象）
|-- linker/                 # Splatが生成するリンカーファイル（gitignore対象）
|-- build/                  # オブジェクト、ELF、map、diff、レポート
|-- dockerfiles/            # 開発用 / CI用コンテナ
|-- Makefile
`-- requirements.txt
```

まず確認するとよい場所:

- `src/` - 復元したゲームコードとPsy-Qコード。
- `asm/nonmatchings/` と `asm/overlays/*/nonmatchings/` - 未一致関数向けに生成されたターゲットアセンブリ。
- `config/symbols/` - 判明している関数 / グローバルのアドレス。
- `config/relocations/` - splatがシンボリック参照を正しく復元できない場合に使用するrelocation override。
- `mk/overlay-registry.mk` - オーバーレイごとのソース / ツールチェーン割り当て。
- `docs/decompilation/` - このプロジェクト固有のマッチング作業メモ。

## ドキュメント

プロジェクト固有の資料として、次のドキュメントがあります。

- [`docs/decompilation/gcc-272-matching-techniques.md`](docs/decompilation/gcc-272-matching-techniques.md)
- [`docs/decompilation/splat-reloc-overrides.md`](docs/decompilation/splat-reloc-overrides.md)
- [`docs/decompilation/psyq-gpu-primitives.md`](docs/decompilation/psyq-gpu-primitives.md)
- [`tools/compressor/README.md`](tools/compressor/README.md)

## トラブルシューティング

**`make splat` でファイル不足またはSHA-1不一致が報告される**

現在のビルドには北米版が必要です（日本版にも近日対応予定です）。北米版からファイルを抽出し、名前を変更せずに `disc/SLPS_021.70` と `disc/BIN/*.BIN` へ配置していることを確認してください。

**Dockerが `old-gcc/...` イメージを見つけられない**

Gitサブモジュールを初期化し、`lom-dev` をビルドする前に、セットアップ手順にある4つの古いコンパイライメージをビルドしてください。

**GCCで `Value too large for defined data type` と表示される**

`/lom` から直接コンパイルせず、Makefileを使用してください。ステージングされたツリーを更新する必要がある場合は `make recopy` を実行します。

**シンボル / 設定の変更が生成されたアセンブリへ反映されない**

`make splat` を再実行してください。生成された `asm/` や `linker/` のファイルを手動で編集しないでください。

**別のコンパイラでは関数が一致するのに、プロジェクトのビルドでは一致しない**

`mk/main.mk` または `mk/overlay-registry.mk` で、そのソースがどのツールチェーンへ振り分けられているか確認してください。プロジェクトに設定されている当時のツールチェーンが基準です。

**objdiffでは100%なのに、オーバーレイ全体の検証に失敗する**

data / rodata内のジャンプテーブルやcaseのターゲット、relocation addend、リンカーのセクション順序、オーバーレイセグメントの `align:` キー、生成アセットなどを確認してください。

## GitHub Issues

不具合の報告、質問、要望などは [GitHub Issues](https://github.com/celophi/lom-decomp/issues) からお気軽にどうぞ。

メンテナーは日本語をある程度理解でき、話すこともできますので、Issueは**日本語で書いていただいて大丈夫です**。英語で書く必要はありません。返信は日本語と英語の両方で行います。

## リバースエンジニアリングの出所

本プロジェクトは、一般に販売された製品版『Legend of Mana』を独自に解析し、逆アセンブル、デコンパイル、バイナリ比較、実行時解析、および一般公開されている技術資料やツールを用いて、その挙動と機械語を復元したものです。

本プロジェクトの作成にあたり、流出したものを含む非公開の **『Legend of Mana』** ソースコード、デバッグシンボルファイル、内部シンボルマップ、開発資料、その他SquareまたはSquare Enixの機密資料は使用していません。

特に記載がない限り、関数名、変数名、データ構造、翻訳単位の境界、その他のソースコード上の情報は、製品版バイナリと観測された挙動から復元または推定したものです。オリジナル開発時の名称や構成と同一であるとは限りません。

このリポジトリに、流出したソースコード、非公開のデバッグシンボル、機密の開発資料、その他オリジナルゲームの開発に由来する非公開資料を含めることは**今後もありません**。

## 法的事項

このリポジトリは、独立したリバースエンジニアリングおよび保存活動のためのプロジェクトです。Square、Square Enix、Sony、その他の権利者とは関係がなく、承認を受けたものでもありません。

オリジナルのゲーム実行ファイル、オーバーレイバイナリ、アートワーク、音声、その他の著作権で保護されたゲームデータを、このリポジトリへコミットしないでください。必要なデータは、正規に入手した各自のゲームから用意する必要があります。

**『Legend of Mana』** および関連する名称・アセットの権利は、それぞれの権利者に帰属します。

## 謝辞

Squaresoft、そして **『Legend of Mana』** の制作に携わったすべての方々に、心から感謝します。このゲームには豊かな想像力、実験的な試み、独創的なアイデア、美しいアートと音楽、そして発売から何十年経った今でも解析していて面白い技術的な工夫が詰まっています。このようなプロジェクトが存在するのは、当時の開発者、アーティスト、音楽家、デザイナー、ライター、サポートスタッフの皆さんが挑戦を重ね、今なお多くの人が理解し、残していきたいと思えるほど個性的な作品を生み出してくださったからです。

何より、このデコンパイルはその仕事への敬意と感謝を形にしたものです。これほど美しく記憶に残るゲームを作ってくださったこと、そして他とは違うものを作ることに挑戦してくださったことに、あらためて感謝します。

## 使用ツールと謝辞

本プロジェクトは、より広いデコンパイルコミュニティによるツールや研究成果の上に成り立っています。主に次のプロジェクトを利用しています。

- [splat](https://github.com/ethteck/splat)
- [spimdisasm](https://github.com/Decompollaborate/spimdisasm)
- [maspsx](https://github.com/mkst/maspsx)
- [old-gcc](https://github.com/decompals/old-gcc)
- [objdiff](https://github.com/encounter/objdiff)
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter)
- [m2c](https://github.com/matt-kempster/m2c)
- [wibo](https://github.com/decompals/wibo)
- [psyq-obj-parser](https://github.com/mkst/psyq-obj-parser)
- [decomp.me](https://decomp.me)
- [decomp.dev](https://decomp.dev)
