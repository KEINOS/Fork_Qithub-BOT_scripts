<?php
/**
 * 文字列データのREAD/WRITE/DELETEを行います
 *
 * DBの使えないサーバーなので、ファイルベースで管理しています。
 *
 * 【このスクリプトの Qithub API パラメーター】
 *
 * ■受け取り必須項目
 *     'command' => string 'save','load','delete'（必須）
 *     'id'      => string 保存するデータのキー（必須）
 *     'value'   => string
 *
 * ■出力項目
 *     'result' => string 'OK','NG'
 *     'value'  => string
 *
 */

/* =====================================================================
    Main
   ===================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 定数
define('DIR_NAME_DATA', 'data');
define('DIR_SEP', DIRECTORY_SEPARATOR);

// 標準入力を取得
$arg = get_api_input_as_array();

// 必須項目のチェック
if (! isset($arg['command']) or ! isset($arg['id'])) {
    $temp = print_r($arg, true);

    die( encode_array_to_api([
        'result' => 'NG',
        'value'  => "'command'と'id'は必須項目です。\n${temp}",
    ]));

} else {
    $id      = urlencode($arg['id']); //Ensure as filename
    $command = $arg['command'];
    $data    = $arg['value'] ?: '';
    $path_dir_current = dirname(__FILE__) . DIR_SEP;
    $path_dir_data    = $path_dir_current . DIR_NAME_DATA . DIR_SEP;
    $path_file_data   = $path_dir_data . $id . '.dat';
    
    switch($command){
        case 'load':
            if (file_exists($path_file_data)) {
                $result = 'OK';
                $value  = file_get_contents($path_file_data);
                $value  = unserialize($value);
            } else {
                $result = 'NG';
                $value  = 'No data saved.';
            }
            break;
        case 'save':
            // データディレクトリのチェック
            if (! dir_exists($path_dir_data)) {
                if (mkdir($path_dir_data, 0705)) {
                    touch($path_dir_data . 'index.html');
                }
            }
            // データの保存
            $data   = serialize($data);
            $result = (file_put_contents($path_file_data, $data))? 'OK' : 'NG';
            $value  = $data;
            break;
        case 'delete':
            if (file_exists($path_file_data) && unlink($path_file_data)) {
                $result = 'OK';
                $value  = 'Data deleted successfully.';
            } else {
                $result = 'NG';
                $value  = 'Can not delete data.';
            }
            break;
        default:
            $result = 'NG';
            $value  = "Invalid command: ${command}";
            break;
    }

    echo encode_array_to_api([
        'result' => $result,
        'value'  => $value,
    ]);
}


die();

/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */
/**
 *  配列をJSON & URLエンコードにエンコードします
 *
 *  'system'および'plugins'は、このデータ形式で入力を受け付けます。
 *
 * @param  array  $array_value スクリプトの実行言語（'php','python'）
 * @return string              エンコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function encode_array_to_api($array_value)
{
    $json_raw = json_encode($array_value);
    $json_enc = urlencode($json_raw);

    return $json_enc;
}

function get_api_input_as_array()
{
    return json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
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

function dir_exists($path_dir)
{
    return is_dir($path_dir);
}
