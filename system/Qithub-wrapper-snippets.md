# Qithub API の各種言語のラッパーのスニペット一覧

本体スクリプト（`Qithub-BOT/scripts/index.php`。以下、本体スクリプト）から受け取るデータと、その処理結果を表示するための仕様に準拠したユーザー関数の各種言語のスニペット一覧です。

基本的には、下記概要を行うものなので、準拠していれば各自で実装して構いません。

## 概要

### Qithub エンコードとは

> JSON 形式の文字列を URL エンコードした１文字列のこと。

Qithub では、本体スクリプトとユーザー・スクリプト（ `Qithub-BOT/scripts/plugins/` もしくは `Qithub-BOT/scripts/system/` 下に設置されたスクリプト）間は、この Qithub エンコードされた１つの文字列でデータの送受を行います。

### データの受け取り（INPUT）

- ユーザー・スクリプトは本体スクリプトから CLI 経由で呼び出し（実行）されます
- ユーザー・スクリプトが必要なデータは、本体スクリプトからの呼び出し（実行）時の**第１引数に Qithub エンコードされた文字列で渡されます**

そのため、ユーザー・スクリプトは以下の処理が必須となります。

1. 標準入力より第１引数の文字列の取得 = Qithub エンコードされたデータの受け取り
2. 取得した文字列の URL デコード = JSON文字列へ変換

なお、受け取った JSON 文字列を内部処理用に配列やオブジェクトへ変換するのは、各ユーザー・スクリプトの自由です。

### 処理結果の差し出し（OUTPUT）

本体スクリプトから呼び出された各ユーザー・スクリプトは、処理結果と処理内容を、以下の JSON 配列に代入し URL エンコードした文字列として標準出力します。

```
[
    "result" : "[your_status]",
    "value"  : "[your_contents]"
]
```

|キー名|値の形式|概要|説明|
|:--- | :---: | :--- | :--- |
| `result` | string | 処理結果 | "OK" / "NG" のいずれかの文字列。ユーザー・スクリプト内の処理が正常に行われた場合は "OK" にすること。|
| `value`  | string | 処理内容 | "result" のステータスが "NG" の場合は、そのエラー内容。"OK" の場合は、処理結果の値を返します。JSON形式 か文字列かは各ユーザー・スクリプトの仕様によります。各スクリプトの README.md もしくは main スクリプト内に明記すること|

---

## PHPスニペット

### INPUT

```functions.php
<?php

/**
 * get_api_input_as_array function.
 * 標準入力より Qiita API 準拠で受け取った Qithub エンコードデータを PHP 配列で返す。
 *
 * @access public
 * @return array
 * @ver 20171220
 */
function get_api_input_as_array()
{
    return json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
}

/**
 * get_api_input_as_json function.
 * 標準入力より Qiita API 準拠で受け取ったデータを Qithub エンコードデータを JSON文字列で返す。
 *
 * @access public
 * @return string JSON format string
 * @ver 20171220
 */
function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

/**
 * get_stdin_first_arg function.
 * 標準入力の第１引数返す。（Qithub API 準拠）
 *
 * @access public
 * @return string
 * @ver 20171220
 */
function get_stdin_first_arg()
{
    // CLIからの標準入力を取得
    global $argv;

    // 引数１は必須
    if (empty($argv[1])) {
        print_output_as_api('NG', 'Argument is empty.');
    }

    return trim($argv[1]);
}
```

### OUTPUT

```functions.php
<?php

/**
 * print_output_as_api function.
 * 処理結果を Qithub の スクリプト間 API に準拠して標準出力する
 * 
 * @access public
 * @param  string $status    "OK" or "NG" の文字列
 * @param  string $contents  処理結果
 * @return string
 */
function print_output_as_api($status, $contents)
{
    $status = (string) $status;
    $status = trim( $status );
    $status = ( 'ok' == $status ) ? 'OK' : $status;

    $contents = (string) $contents;

    $array = [
        'result' => $status,
        'value'  => $contents,
    ];

    echo encode_array_to_api($array);
    die();
}

/**
 * encode_array_to_api function.
 * PHP 配列を Qithub エンコードに変換する。
 *
 * @param  array  $array 
 * @return string              Qithub エンコード文字列
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function encode_array_to_api($array)
{
    $json_raw = json_encode($array);
    $json_enc = urlencode($json_raw);

    return $json_enc;
}

```

### Sample usage

```main.php
/* ============================================
 *   おうむ返しのサンプル・プラグイン
 * ============================================
 * Usage : 受け取ったデータをそのままJSONで返します。
 * Require : none
 * Return value: json string
 */

// Input
$input_array = get_api_input_as_array();

// 処理のサンプル
$contents_array  = $input_array;
$contents_string = json_encode( $contents_array );

// Output
$status = 'OK';

print_output_as_api( $status, $contents_string );

```

---


