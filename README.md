# このリポジトリについて

このリポジトリは、[Qiita](https://qiita.com/)／[Qiitadon](https://qiitadon.com/)のコラボレーション支援BOT 『**Qithub**』の本体スクリプトを、**Qiitaユーザで共同開発するためのリポジトリ**です。

- 『Qithub』のプロジェクトサイト： https://github.com/Qithub-BOT/
- 『Qiitaコラボ記事』（Qiitaのユーザ間で共同編集可能な記事）に関しては[`items`リポジトリ](https://github.com/Qithub-BOT/items)をご覧ください。

## この BOT でできること （2017/10/03 現在）

1. cronによる定例処理
2. プラグインの実行

## この BOT の機能について

- Qiitaコラボ記事（ xxxxxxxxx @ Qiita もしくは xxxxxxxxx @ GitHub）を参照ください。

## BOT のプラグインについて

本体スクリプト（"scripts/[index.php](https://github.com/Qithub-BOT/scripts/blob/master/index.php)"）は、各プログラム言語で書かれたプラグインをCLI（Command Line Interface)を通して実行します。

詳しい仕様に関しては Qiitaコラボ記事（ yyyyyyyyy @ Qiita もしくは yyyyyyyyy @ GitHub）をご覧ください。

各プラグインは`scrpts/_scripts/`ディレクトリ下に「プラグインID」をディレクトリ名として設置されます。
また、「プラグインID」は、原則として`scripts`リポジトリの[issue](https://github.com/Qithub-BOT/scripts/issues?utf8=%E2%9C%93&q=is%3Aissue%20)にて提案・要望されたissue番号と同等です。

例えば [issue#10] で提案された「Hello worldをトゥートするだけの機能」は、 "[scripts / _scripts / issue10](https://github.com/Qithub-BOT/scripts/tree/master/_scripts/issue10) / [hello_world.php](https://github.com/Qithub-BOT/scripts/blob/master/_scripts/issue10/hello_world.php)" に設置されています。

各プラグインは直接実行されることはなく、本体スクリプトからコマンドラインで実行される形で呼び出されますが、標準入力から渡された引数のデータを元に単体で動作／完結し、実行結果はスルーされ（`dev/null`に渡され）ます。

