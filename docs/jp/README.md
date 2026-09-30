# 日本語ドキュメント

[ドキュメント一覧](../README.md) | [プロジェクトのREADME](../../README_JP.md) | [English](../en/README.md)

ゲームの仕組みを、具体例とソースコードを使って説明します。

## ゲームの仕組み

- [シーンのレイアウトと出現条件](technical/architecture/scene-layouts.md) - シーン、レイアウト、レコードの関係と、宝箱を例にした条件の読み方。
- [CHECKPSオーバーレイ](technical/architecture/checkps.md) - 起動画面、日本版のCDチェック、ドライブの引き継ぎと改造の警告。
- [CARDAオーバーレイ](technical/architecture/carda.md) - セーブ、ロード、PocketStationとのペットのやり取り。カード画面からFIELDへ戻るまでの流れ。
- [ADDHEROオーバーレイ](technical/architecture/addhero.md) - 2Pの主人公の画面。友達の主人公をメモリーカードから読み込み、書き戻す仕組み。

## 関連資料

- [シーン抽出ツール](technical/reference/scene-extractor.md) - シーンIMGの抽出方法、YAMLの項目、元のバイト列の保持について。
- [ZUKANのリソース](technical/reference/zukan-resources.md) - 図鑑画面のテクスチャ、UIスプライト、項目名、分類テーブルの抽出。
- [WSELのリソース](technical/reference/wsel-resources.md) - プレイエリア選択画面の画像、スプライトレイヤー、主人公のポーズ、土地マップのグリッド表の抽出。
- [WMAPのリソース](technical/reference/wmap-resources.md) - ワールドマップのテーブル、効果音、自動入力スクリプト、ステップテーブルの抽出。
- [TITLEのリソース](technical/reference/title-resources.md) - タイトルメニューの画像、キャラクターと武器の選択画面、初期武器、ニューゲームの状態の抽出。
- [SHOPのリソース](technical/reference/shop-resources.md) - 店のアイテムテキスト、装備の種別名、楽器の魔法名、売値テーブルの抽出。
- [NIKIのリソース](technical/reference/niki-resources.md) - 日記のセーブ画面のテキスト、パーティーアイコン、カード処理シーケンス、文字表の抽出。
- [MENUのリソース](technical/reference/menu-resources.md) - ゲーム内メニューのテクスチャ、アイコン、テキストテーブル、ページ配置、入力スクリプトの抽出。
- [GOSUBのリソース](technical/reference/gosub-resources.md) - 作成・育成画面のテキスト、ポートレート、UIグリフ、装備分類テーブルの抽出。
- [GOLEMのリソース](technical/reference/golem-resources.md) - ロジックグリッドの画像、ブロック名と説明、グリフ、パネルレコードの抽出。
- [GNAMEのリソース](technical/reference/gname-resources.md) - 名前入力画面の画像、文字パネル、名前リスト、配置テーブルの抽出。
- [FIELDのリソース](technical/reference/field-resources.md) - 常駐する画像、テキスト、アニメーションデータ、ゲーム用テーブルの抽出。
- [CLOADのリソース](technical/reference/cload-resources.md) - ロード画面のテキスト、カード処理シーケンス、文字表の抽出。
- [ディスクの構成](technical/reference/disc-layout.md) - 北米版のファイル構成、リソース番号、ディスク上の位置の指定方法。
- [オーバーレイのIDプレフィックス](technical/reference/overlay-id-prefix.md) - 先頭の4バイトの意味、配置の根拠、復元したコードでの実装。
- [セーブファイルの形式](technical/reference/save-file.md) - メモリーカードのセーブの中身と、チェックサムの直し方。
- [テキストテーブル](technical/reference/text-tables.md) - メニューのテキストの格納場所と、北米版のテキストの文字コード。
- [英語ドキュメント一覧](../en/README.md) - その他の技術解説とリファレンス。
