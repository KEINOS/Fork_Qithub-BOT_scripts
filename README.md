# このリポジトリについて

このリポジトリは、[Qiita](https://qiita.com/)／[Qiitadon](https://qiitadon.com/)のコラボレーション支援BOT 『**Qithub**』の本体スクリプトを、**Qiitaユーザで共同開発するためのリポジトリ**です。

- 『Qithub』のプロジェクトサイト： https://github.com/Qithub-BOT/
- 『Qiitaコラボ記事』（Qiitaのユーザ間で共同編集可能な記事）に関しては[`items`リポジトリ](https://github.com/Qithub-BOT/items)をご覧ください。

## このBOTでできること （2017/10/03 現在）

1. cronによる定例処理
2. プラグインの実行

## プラグインについて

本体スクリプト（"scripts/[index.php](https://github.com/Qithub-BOT/scripts/blob/master/index.php)"）は、各プログラム言語で書かれたプラグインをCLI（Command Line Interface)を通して実行します。

各プラグインは`scrpts/_scripts/`ディレクトリ下に「プラグインID」をディレクトリ名として設置されます。
「プラグインID」は、原則として`scripts`リポジトリの[issue](https://github.com/Qithub-BOT/scripts/issues?utf8=%E2%9C%93&q=is%3Aissue%20)にて提案・要望されたissue番号と同等です。

例えば [issue#10] で提案された「Hello worldをトゥートするだけの機能」は、 "[scripts / _scripts / issue10](https://github.com/Qithub-BOT/scripts/tree/master/_scripts/issue10) / [hello_world.php](https://github.com/Qithub-BOT/scripts/blob/master/_scripts/issue10/hello_world.php)" に設置されています。

各プラグインは直接実行されることはなく、本体スクリプトからコマンドラインで実行される形で呼び出されますが、標準入力から渡された引数のデータを元に単体で動作／完結し、実行結果はスルーされ（`dev/null`に渡され）ます。

### 実装済みのプラグイン

| プラグインID | スクリプト名 | 開発言語　| 概要 | 定例処理対象 | 備考 |
| --- | --- | :---: | --- | :---: | --- |
| [issue10][issue#10] | hello_world.php | PHP | 「Hello world!」を　トゥートし、前回のトゥートを削除する　| ◯ | 定例処理の対象から外す予定 |

[issue#10]:https://github.com/Qithub-BOT/scripts/issues/10

### プラグインの基本仕様

1. 本体スクリプトからプラグインが呼び出される（CLIで実行される）と、プラグインは以下の**情報を標準入力の第一引数から受け取れます**。
    - Mastodon/QiitaのAPI操作のためのアクセストークン
    - 本体スクリプトが受け取ったGET（クエリ）情報
    - 本体スクリプトが受け取ったPOST情報
1. 上記で**受け取るデータはURLエンコードされたJSONデータ**です。(UTF-8)

### プラグイン作成の制約次項

1. 本体スクリプトから呼び出される（実行される）スクリプト名は特に制限を設けていません。
1. プラグインは自身のプラグイン・ディレクトリ以外にファイルを作成してはいけません。
1. データファイルなど、プラグインが自動作成するファイルがある場合は、自身のプラグイン・ディレクトリ直下に".gitignore"で対象外にしてください。

