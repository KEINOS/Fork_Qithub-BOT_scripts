<?php
/**
 * 指定したサイコロを振った結果を返すスクリプト
 *
 * @copyright Qithub Organization All Rights Reserved
 * @license 2017/11/05 under discussion
 * @author @hidao80 (@hidao@Qiitadon.com)
 * @link https://github.com/Qithub-BOT/
 * @php version >=5.5.9-1
 */

/* =====================================================================
    Main
   ===================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 標準入力を取（テストコード）
//global $argv;
//$arg = array("dicecode" => $argv[1]);

// 標準入力を取得
$arg = get_api_input_as_array();

// ダイスコードを取得
$dice_code = $arg['dicecode'];

// メッセージの作成（RAW）
$msg_raw = dice_roll($dice_code);

// API準拠の出力結果作成
$msg_api = [
    'result' => 'OK',
    'value'  => $msg_raw
];
$msg_enc = json_encode($msg_api);
$msg_enc = urlencode($msg_enc);

// プラグインの処理結果を出力
echo $msg_raw."\n";
echo $msg_enc;

die();

/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */
function get_api_input_as_array()
{
    return json_decode( get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
}

function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

function get_stdin_first_arg()
{
    // CLIからの標準入力を取得
    global $argv;

    // 引数１は必須
    if (empty($argv[1])) {
        print_r($argv);
        die("Argument is empty.");
    }

    return trim($argv[1]);
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

/**
 * Decode and curiculating dice code
 */
function dice_roll($dice_code)
{
    $param = explode("D", strtoupper(trim($dice_code)));
    
    $sum = 0;
    $result = array();
    $times = intval($param[0]);
    $max = intval($param[1]);
    for ($i = 0; $i < $times; $i++) {
      $result[] = rand(1, $max);
      $sum += end($result);
    }
    
    return "Result: ".implode(" ", $result)."\nSum: ".$sum;
}