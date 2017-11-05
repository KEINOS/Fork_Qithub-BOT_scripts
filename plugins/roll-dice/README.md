# plugins/roll-dice

指定したサイコロを振った結果を返すスクリプト

このスクリプトが呼び出される（リクエストされる）と、メッセージとしてサイコロを振った結果とその合計を生成しレスポンスとして返します。

また、リクエスト時に「dice_code」にダイスコードを（'ndm'のフォーマットで）パラメーターとして指定すると、n をダイスの数、m をダイスの目の最大値としてダイスを振り、その結果と合計をメッセージとして返します。

例）
    $param_request = [
        'is_mode_debug' => false,
        'dice_code' => '2d6', //6つ目のサイコロを2つ振った場合
    ];

| プログラム言語 | 呼び出しスクリプト名 | 関連issue番号 |
| --- | --- | :---: |
| PHP | plugins/roll-dice | [#38](https://github.com/Qithub-BOT/scripts/pull/38), [#43](https://github.com/Qithub-BOT/scripts/issues/43) |

## 動作実績

| 言語バージョン | サーバー |
| --- | --- |
| PHP 5.5.9-1ubuntu4.22 (cli) | c9.io |

## 動作仕様

1. 本体スクリプトより CLI (Command Line Interface) 経由で実行
2. 第１引数から Qithub API フォーマットでデータを受け取る（下記 Request query parameters参照）
3. 受け取ったデータのうち'dicecode'（'ndm'のパラメーター）に値がある場合はダイス数とダイスの目の数を変更。指定がない場合は '1d6' とする。
4. Qithub API フォーマットで処理結果を出力（下記 Responce parameters参照）

### Request query parameters:

| Field         | Type   | Required | Description |
| ------------- | ------ | --- | ----------- |
| is_mode_debug | bool   | yes | 本体スクリプトがデバッグモードで稼働しているかの受け取りフラグ |
| dice_code     | string | no  | (Default: '1d6') ダイス数と最大値を指定する文字列をダイスコードで表す |

### Responce parameters:

| Field  | Type   | Description |
| -------| ------ | ----------- |
| result | OK/NG  | 正常処理の場合は文字列'OK'が、内部処理に問題があった場合は文字列'NG'が返されます。 |
| value  | string | 処理結果の文字列が返されます。エラーの場合は、エラー内容。 |
