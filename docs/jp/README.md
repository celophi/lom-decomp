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
- [ディスクの構成](technical/reference/disc-layout.md) - 北米版のファイル構成、リソース番号、ディスク上の位置の指定方法。
- [オーバーレイのIDプレフィックス](technical/reference/overlay-id-prefix.md) - 先頭の4バイトの意味、配置の根拠、復元したコードでの実装。
- [セーブファイルの形式](technical/reference/save-file.md) - メモリーカードのセーブの中身と、チェックサムの直し方。
- [テキストテーブル](technical/reference/text-tables.md) - メニューのテキストの格納場所と、北米版のテキストの文字コード。
- [英語ドキュメント一覧](../en/README.md) - その他の技術解説とリファレンス。
