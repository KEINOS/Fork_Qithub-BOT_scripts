# このリポジトリについて

このリポジトリは、[Qiita](https://qiita.com/)／[Qiitadon](https://qiitadon.com/)のコラボレーション支援BOT 『**Qithub**』の本体スクリプトを、**Qiitaユーザで共同開発するためのリポジトリ**です。

- 『Qithub』のプロジェクトサイト： https://github.com/Qithub-BOT/
- 『Qiitaコラボ記事』（Qiitaのユーザ間で共同編集可能な記事）に関しては[`items`リポジトリ](https://github.com/Qithub-BOT/items)をご覧ください。

## この BOT の目的

- 新しく覚えたい言語や得意な言語で楽しく気軽に開発やコラボをする場として
- GitHub でコントリビュート（参加）したいが色々と怖いので一緒に気軽にトライできる場として
- 面白そうな機能の BOT への実装 [[皮算用機能一覧](https://github.com/Qithub-BOT/items/issues/14)]

## この BOT でできること （2017/11/27 現在）

1. cronによる定例処理
  - 新着Qiita記事のトゥート
2. プラグインの実行

## この BOT の基本仕様について

- 現在は @KEINOS のショボい個人のレンタル・サーバーで動いています
- 本体スクリプト（"scripts/[index.php](https://github.com/Qithub-BOT/scripts/blob/master/index.php)"）は以下の３つの基本機能で動いています。

  1. `plugins/`
    各種言語で書かれた単体で動作完結するスクリプト （サイコロを振って出目を返すなど）

  2. `system/`
    アクセストークンなど BOT の固有情報を使わないと動かない各種言語で書かれたスクリプト （トゥートの実行など）

  3. `includes/proc/`
    `plugins`や`system`などのスクリプトを組み合わせて、より複雑な処理をする（現在PHPのみ対応）

## BOT のプラグインについて

本体スクリプト（"scripts/[index.php](https://github.com/Qithub-BOT/scripts/blob/master/index.php)"）は、各プログラム言語で書かれたプラグインをCLI（Command Line Interface)を通して実行し、トゥートする内容を取得します。

詳しい仕様や質問に関しては [plugins/README.md](./plugins/README.md) をご覧いただくか、このリポジトリの[issue](https://github.com/Qithub-BOT/items/issues?q=is%3Aissue)でお気軽にご質問ください。

## BOT の参加方法

Qiitaのユーザーなら誰でも参加できます。

- リポジトリをクローン／フォークして [PR](https://github.com/Qithub-BOT/items/blob/master/7157c17765e328917667.md) をあげる。（[PR のルール](https://github.com/Qithub-BOT/items/blob/master/1a52282f0b132347c2b1.md#%E9%81%8B%E5%96%B6%E3%83%AB%E3%83%BC%E3%83%AB)を読んでもわかりづらい場合は、遠慮なく [issue](https://github.com/Qithub-BOT/items/issues?q=is%3Aissue) を立てて聞いてください。）

- [issue](https://github.com/Qithub-BOT/items/issues?q=is%3Aissue) で要望を立てる

- [Qiitadon](https://qiitadon.com/) でメンバーに聞いて見る

    - [＠KEINOS](https://qiitadon.com/@KEINOS)

    - [＠hidao](https://qiitadon.com/@hidao)

## 詳しい情報

「[Qithub 設定資料集](https://github.com/Qithub-BOT/items/blob/master/1a52282f0b132347c2b1.md)」をご覧ください。


