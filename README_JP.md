# 聖剣伝説 LEGEND OF MANA（PS版）デコンパイル

（Legend of Mana PSX Decompilation）

**言語:** [English](README.md) | 日本語

[![US Progress]][us progress site]
[![JP Progress]][jp progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[US Progress]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13.svg?mode=shield&measure=code&label=%E5%8C%97%E7%B1%B3%E7%89%88
[us progress site]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13
[JP Progress]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70.svg?mode=shield&measure=code&label=%E6%97%A5%E6%9C%AC%E7%89%88
[jp progress site]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70

PlayStation用ソフト **『聖剣伝説 LEGEND OF MANA』** を、オリジナルと一致する形で復元する**デコンパイル（逆コンパイル）プロジェクト**です。復元した読みやすいCソースコードは、オリジナルのゲームと同じ機械語にコンパイルされます。北米版と日本版のどちらも、同じCソースから**完全リンク**されています。

このプロジェクトはデコンパイルです。リコンパイルでは**ありません**。ソースポートでも**ありません**。このリポジトリには、ゲームの実行ファイル、オーバーレイバイナリ、アートワーク、音声、その他の著作権で保護されたゲームデータは含まれていません。必要なファイルは、正規に入手した各自のゲームから用意してください。

このプロジェクトの目的は、研究とMod制作のために、オリジナルゲームのロジックと挙動を保存することです。

## 現在の状況

北米版と日本版のどちらも、メイン実行ファイルと17個すべてのオーバーレイを含む全モジュールが**完全リンク済み**です。意味は他のデコンパイルプロジェクトと同じですが、本プロジェクトでは条件を1つ追加しています。コンプレッサーの出力が、ディスク上の `.BIN` ファイルとバイト単位で一致しなければなりません。この追加の確認は、進捗を把握するのに役立ちました。本プロジェクトでは、次の2つの条件を満たしたモジュールを完全リンク済みとしています。

1. ビルドによって生成された**ELFのバイト列が、オリジナルを展開したファイルと一致する**こと。
2. そのELFをrawバイナリに変換し、本プロジェクトのコンプレッサーで圧縮すると、**ディスク上の元の `.BIN` ファイルと完全に同一のファイルを再生成できる**こと。

つまり、往復処理はビット単位で完全一致します。

1. ディスク上のオリジナルの `.BIN` から始めます。
2. それを展開します。
3. CソースをコンパイルしてELFを作ります。
4. ELFを圧縮します。
5. 結果は元と同じ `.BIN` になります。

メイン実行ファイルは圧縮されていないため、`SLUS_010.13` と `SLPS_021.70` では1つ目の条件だけが確認対象です。リンク済みELFをrawバイナリへ変換したものが、ディスク上のファイルと一致します。

**（コンプレッサーもぜひ見てみてください。正直ここまでやる必要はないのですが、元のファイルと _ビット単位で完全一致_ するのはかなり面白いです！）**

すべてのモジュールを確認するには `make verify-bins` を、日本版では `make verify-bins VERSION=jp` を実行してください。

## 対応バージョン

| 項目 | 北米版 | 日本版 |
|---|---|---|
| ディスクシリアル | `SLUS-01013` | `SLPS-02170` |
| メイン実行ファイル SHA-1 | `d11dfdd50d412ac3fa3e2eb80fbde138da118f27` | `b067188a92e4de9a4db7bb7e5343c757e9884bfa` |
| ディスクイメージ（`.bin`）SHA-1 | `c1b536c99f0d390584eb30462a7e37f2bbef3902` | `7a314615be8a482cf3f81b4101cc19aaa738f36d` |

その他のリージョンには現在対応していません。

## ドキュメント

[日本語ドキュメント一覧](docs/jp/README.md)に、ゲームの仕組みの解説と関連資料をまとめています。

- [`tools/compression/README.md`](tools/compression/README.md)

## 必要なもの

次のものが必要です。

- **Git**
- **Docker** - Windows/macOSではDocker Desktop、LinuxではDocker Engine。
- **正規に入手した『Legend of Mana』**（北米版または日本版）。
- PlayStationのディスクイメージからファイルを取り出せるツール。

その他に必要なものは、すべて開発用コンテナに含まれています。

## はじめに

以下の手順は日本版を例にしています。北米版では、ファイルを `disc/us/` に置き、`make` コマンドから `VERSION=jp` を外してください。

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

ディスクイメージから、メイン実行ファイルと `BIN` ディレクトリを `disc/jp/` に抽出してください。

```text
disc/
`-- jp/
    |-- SLPS_021.70
    `-- BIN/
        |-- ADDHERO.BIN
        |-- CARDA.BIN
        `-- ...
```

ビルドに必要なのはこれらのファイルだけです。シーン抽出ツールはディスクの `ANA/` ディレクトリも読み込みますが、保存場所はどこでも構いません。パスを指定して実行できます。

> `disc/` はgitignoreの対象です。オリジナルのゲームファイルは絶対にコミットしないでください。

### 3. Dockerイメージをビルドする

リポジトリのルートディレクトリから実行してください。この作業は最初の1回だけで済みます。

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/external/old-gcc/gcc-2.8.0-psx.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/external/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.6.0-psx -f tools/external/old-gcc/gcc-2.6.0-psx.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.7.2-psx-gnu -f dockerfiles/gnu-as.dockerfile tools/external/old-gcc
docker build -t lom-toolchain -f dockerfiles/toolchain.dockerfile .
docker build -t lom-dev -f dockerfiles/dev.dockerfile .
```

### 4. コンテナを起動する

リポジトリのルートディレクトリから実行してください。

PowerShell、bash、zshの場合:

```bash
docker run --rm -it -v "${PWD}:/lom" lom-dev
```

Windows Command Prompt（`cmd.exe`）の場合:

```bat
docker run --rm -it -v "%cd%:/lom" lom-dev
```

これ以降のコマンドは、**コンテナ内で実行します**。

### 5. オリジナルのバイナリを分割する

```bash
make splat VERSION=jp
```

ファイルを期待されるSHA-1ハッシュと照合したうえで、ビルドに必要なアセンブリ、リンカースクリプト、アセットを生成します。

### 6. ビルドして検証する

```bash
make everything VERSION=jp
make verify-bins VERSION=jp
```

`make everything` は、メイン実行ファイルと17個すべてのオーバーレイを `build/jp/` にビルドします。`make verify-bins` は、それらすべてが完全リンク済みであることを確認します。

## トラブルシューティング

**`make splat` でファイル不足またはSHA-1不一致が報告される**

北米版は `disc/us/SLUS_010.13` と `disc/us/BIN/*.BIN`、日本版は `disc/jp/SLPS_021.70` と `disc/jp/BIN/*.BIN` に、名前を変更せずに配置していることを確認してください。配置が正しいのにSHA-1が一致しない場合は、別のリリースのファイルか、吸い出しに失敗している可能性があります。[対応バージョン](#対応バージョン)と比較してください。

**Dockerが `old-gcc/...` イメージを見つけられない**

Gitサブモジュールを初期化し、[はじめに](#はじめに)の手順3に記載された順番でDockerイメージをビルドしてください。

**GCCで `Value too large for defined data type` と表示される**

古い32-bitコンパイラはWindows上のDockerマウント内のファイルを読み込めないため、Makefileはソースをコンテナ内の `/staging` へコピーし、そこでコンパイルします。`/lom` から直接コンパイルせず、Makefileを使用してください。ステージングされたツリーを更新する必要がある場合は `make recopy` を実行します。

## GitHub Issues

不具合の報告、質問、要望などは [GitHub Issues](https://github.com/celophi/lom-decomp/issues) からお気軽にどうぞ。

メンテナーは日本語をある程度理解でき、話すこともできますので、Issueは**日本語で書いていただいて大丈夫です**。英語で書く必要はありません。返信は日本語と英語の両方で行います。

## Makeターゲット

| ターゲット | 用途 |
|---|---|
| `make` | メインの `SLPS_021.70` ELFをビルドします。 |
| `make bin` | `build/jp/SLPS_021.70.bin` も生成します。 |
| `make <overlay>` | `make field` など、オーバーレイを1つビルドします。 |
| `make overlays` | 17個すべてのオーバーレイをビルドします。 |
| `make everything` | メイン実行ファイルと17個すべてのオーバーレイをビルドします。 |
| `make splat` | オーバーレイを展開し、すべてのバイナリをアセンブリ、リンカースクリプト、アセットに分割します。 |
| `make objdiff-objects` | objdiff用にターゲット側と復元側のオブジェクトをビルドします。 |
| `make objdiff-config` | `objdiff.json` を再生成します。objdiffでリポジトリのルートを開くと、関数を比較できます。 |
| `make progress` | `build/<version>/progress.json` を生成します。 |
| `make diff-all` | 設定済みの全ユニットに対してobjdiffを実行します。 |
| `make diff-text` | まだ一致していない関数を `build/<version>/diffs/` 以下に出力します。 |
| `make dump-objs` | コード生成の解析用にビルド済みオブジェクトを逆アセンブルします。 |
| `make validate-assets` | フォーマットを認識するアセットについて、往復変換と検証を行います。 |
| `make verify-main` | リンク済みのメイン実行ファイルが `disc/jp/SLPS_021.70` と一致するか確認します（`verify-slus` は別名）。 |
| `make verify-<overlay>` | `make verify-field` など、オーバーレイを1つディスク上のファイルと照合します。 |
| `make verify-bins` | メイン実行ファイルと17個すべてのオーバーレイを、ディスク上のファイルと照合します。 |
| `make verify-compressor` | 選択したバージョンのオリジナルの17個のオーバーレイファイルに対してコンプレッサーを検証します。 |
| `make recopy` | ソース / 設定ファイルを `/staging` へ強制的に再コピーします。 |
| `make clean` | ビルド出力と `/staging` を削除します。 |

すべてのターゲットは、既定では北米版（`VERSION=us`）をビルドします。日本版は `make VERSION=jp` のように `VERSION=jp` を付けて指定します。表中のファイル名は日本版の例です。

ツールのテストやデータ抽出用のターゲットは [tools/README.md](tools/README.md) に記載しています。

## リポジトリ構成

```text
lom-decomp/
|-- src/
|   |-- common/        # 複数のオーバーレイで共有するコード
|   |-- main/          # メイン実行ファイル
|   |-- overlays/      # 17個のオーバーレイ
|   `-- psyq/          # Psy-Q SDKライブラリ
|-- include/           # ヘッダ
|-- config/<version>/  # Splat設定、シンボル、relocation
|-- mk/                # ビルドルール
|-- tools/             # ビルド、データ、検証用ツール
|-- docs/              # アーキテクチャとリファレンスのドキュメント
|-- dockerfiles/       # 開発用 / CI用イメージ
`-- disc/<version>/    # 手元のゲームファイル（gitignore対象）
```

`asm/`、`linker/`、`assets/`、`build/` はバージョンごとに生成され、gitignoreの対象です。

## コンパイラとアセンブラのツールチェーン

このプロジェクトで特に重要なのは、**すべてのソースファイルが同じコンパイラ設定でビルドされているわけではない**という点です。

ちなみに、この複数のツールチェーンへの対応にはかなり「楽しませてもらいました」。特にGNU版はなかなか強敵でした。

ビルドでは、4種類の古いコンパイラビルドを使い、合計7種類のパイプラインを定義しています。コンパイラとアセンブラのフラグは [`mk/toolchains.mk`](mk/toolchains.mk) にあります。ソースの振り分けは、メイン実行ファイルについては [`mk/main-sources.mk`](mk/main-sources.mk)、オーバーレイについては [`mk/overlay-registry.mk`](mk/overlay-registry.mk) にあります。メイン実行ファイルのファイル単位のオーバーライドは [`mk/main.mk`](mk/main.mk) にあり、[`mk/overlays.mk`](mk/overlays.mk) がオーバーレイ用の各バリアントを適用します。

| パイプライン | コンパイラフラグ | アセンブル経路 |
|---|---|---|
| GCC 2.8.0 G0（デフォルト） | `-O2 -G0 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G0、最適化なし | `-O0 -G0 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、division展開あり |
| GCC 2.8.0 G4、division展開なし | `-O2 -G4 -gcoff -fsigned-char` | maspsx、ASPSX 2.77、divisionをそのまま使用 |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx、ASPSX 2.67、division展開あり |
| GCC 2.7.2 GNU G0 | `-O2 -G0` | 当時のGNU `as`（`-O -EL`） |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx、ASPSX 2.34、division展開あり |

## リバースエンジニアリングの出所

本プロジェクトは、一般に販売された製品版『Legend of Mana』を独自に解析し、逆アセンブル、デコンパイル、バイナリ比較、実行時解析、および一般公開されている技術資料やツールを用いて、その挙動と機械語を復元したものです。

本プロジェクトの作成にあたり、流出したものを含む非公開の **『Legend of Mana』** ソースコード、デバッグシンボルファイル、内部シンボルマップ、開発資料、その他SquareまたはSquare Enixの機密資料は使用していません。

特に記載がない限り、関数名、変数名、データ構造、翻訳単位の境界、その他のソースコード上の情報は、製品版バイナリと観測された挙動から復元または推定したものです。オリジナル開発時の名称や構成と同一であるとは限りません。

このリポジトリに、流出したソースコード、非公開のデバッグシンボル、機密の開発資料、その他オリジナルゲームの開発に由来する非公開資料を含めることは**今後もありません**。

## 法的事項

ライセンスの詳細は、[LICENSE](LICENSE) と [第三者に関する通知](THIRD_PARTY_NOTICES.md) を参照してください。

MITライセンスは、本プロジェクトのために独自に作成された成果物に対し、貢献者がその成果物をライセンスするために必要な権利を有する範囲で適用されます。このライセンスは、オリジナルのゲームや第三者のSDK、ライブラリ、開発用ソフトウェアに由来する既存の著作物に対する権利を許諾するものではありません。

このリポジトリは、独立したリバースエンジニアリングおよび保存活動のためのプロジェクトです。Square、Square Enix、Sony、その他の権利者とは関係がなく、承認を受けたものでもありません。

オリジナルのゲーム実行ファイル、オーバーレイバイナリ、アートワーク、音声、その他の著作権で保護されたゲームデータを、このリポジトリへコミットしないでください。必要なデータは、正規に入手した各自のゲームから用意する必要があります。

**『Legend of Mana』** および関連する名称・アセットの権利は、それぞれの権利者に帰属します。

## 謝辞

Squaresoft、そして **『Legend of Mana』** の制作に携わった皆さん、ありがとうございます。アートや音楽、あちこちにあるちょっと変わったアイデアのおかげで、何度でも遊びたくなるゲームです。いろいろなことを試そうとしたチームの姿勢が伝わってきますし、それがこのゲームの面白さにつながっていると思います。

その仕組みをひとつずつ調べていくのは、とても楽しいです。まだわからないことはたくさんありますが、このプロジェクトが皆さんの仕事を残す助けになり、ほかの人にも中身を探ってもらうきっかけになればと思っています。

## 使用ツールと謝辞

本プロジェクトは、より広いデコンパイルコミュニティによるツールや研究成果の上に成り立っています。主に次のプロジェクトを利用しています。

- [splat](https://github.com/ethteck/splat)
- [spimdisasm](https://github.com/Decompollaborate/spimdisasm)
- [maspsx](https://github.com/mkst/maspsx)
- [old-gcc](https://github.com/decompals/old-gcc)
- [objdiff](https://github.com/encounter/objdiff)
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter)
- [m2c](https://github.com/matt-kempster/m2c)
- [decomp.me](https://decomp.me)
- [decomp.dev](https://decomp.dev)
