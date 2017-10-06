<?php
/**
 * KISS から始まるストーリー PHP編
 *
 * "Hello World! 時刻" をトゥート＆前回トゥートを削除するだけのスクリプト
 *
 * 詳しくは同階層の README.md を参照
 *
 */

/* =====================================================================
    Main
   ================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');


// 引数のデコードとAPIキー（アクセストークンなど）を取得
//$arg = json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);

// 鸚鵡返し
echo get_api_input_as_json();

die();

/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */

function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

function get_stdin_first_arg()
{
    // CLIからの標準入力を取得
    $arg = $argv;

    // 引数１は必須
    if (empty($arg[1])) {
        print_r($arg);
        die("Argument is empty.");
    }

    return trim($arg[1]);
}

/* ---------------------------------------------------------------------
    Miscellaneous Functions
   ------------------------------------------------------------------ */

/**
 * Set language to Japanese UTF-8 and Time zone to Japan
 */
function set_utf8_ja($timezone = 'Asia/Tokyo')
{
    if (! function_exists('mb_language')) {
        die('This application requires mb_language.');
    }

    date_default_timezone_set($timezone);
    setlocale(LC_ALL, 'ja_JP');
    mb_language('ja');
    mb_internal_encoding('UTF-8');
    mb_http_output('UTF-8');
    ob_start("mb_output_handler");
}
