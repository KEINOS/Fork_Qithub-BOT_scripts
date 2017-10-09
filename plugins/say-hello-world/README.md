# plugins/say-hello-world

 'Hello World!'を返すだけのスクリプト

このスクリプトが呼び出される（リクエストされる）と、メッセージ「Hello
 World!」を生成しレスポンスとして返します。
 また、リクエスト時に「say_also」パラメーターが指定されていると、その内容
 がメッセージに加えられたものが返されます。

| プログラム言語 | 呼び出しスクリプト名 | 関連issue番号 |
| --- | --- | :---: |
| PHP | plugins/say-hello-world | [issue #10](https://github.com/Qithub-BOT/scripts/issues/10) |

## 動作実績

| 言語バージョン | サーバー |
| --- | --- |
| PHP 7.0.22 (cli) | Xrea @ ValueDomain |

## 動作仕様

1. 本体スクリプトより CLI (Command Line Interface) 経由で実行
2. 第１引数から Qithub API フォーマットでデータを受け取る（下記 Request query parameters参照）
3. 'say_also' パラメーターに値がある場合は 'Hello world!'に追加
4. Qithub API フォーマットで処理結果を出力（下記 Responce parameters参照）

### Request query parameters:

| Field         | Type   | Required | Description |
| ------------- | ------ | --- | ----------- |
| is_mode_debug | bool   | yes | 本体スクリプトがデバッグモードで稼働しているかの受け取りフラグ |
| say_also      | string | no  | "Hello World!"に追加する文字列 |

### Responce parameters:

| Field  | Type   | Description |
| -------| ------ | ----------- |
| result | OK/NG  | 正常処理の場合は文字列'OK'が、内部処理に問題があった場合は文字列'NG'が返されます。 |
| value  | string | 処理結果の文字列が返されます。エラーの場合は、エラー内容。 |


