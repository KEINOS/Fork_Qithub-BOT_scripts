<?php
/**
 * 新規トゥートを行います
 *
 *
 *
 * 【このスクリプトの Qithub API パラメーター】
 *
 * ■受け取り必須項目
 *   必須項目
 *     'domain'       => string トゥート先のドメイン名(ホスト名のみ)
 *     'access_token' => string トゥートに必要なアクセストークン
 *     'status'       => string トゥート内容
 *     'visibility'   => string 'public','unlisted','private','direct'
 *   オプション項目
 *     'spoiler_text' => string この項目があると'CW'モードになり'status'
 *                              は「もっと読む」内の非表示領域になります。
 *
 * ■出力項目
 *     'result' => string 'OK','NG'
 *     'value'  => json    Mastodonからのレスポンスをそのまま返す
 *
 */

/* =====================================================================
    Main
   ===================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 標準入力を取得
$arg = get_api_input_as_array();

// 必須項目を満たしていた場合の処理
if (is_requirement_complied($arg)) {
    $toot_msg  = "#test #newapi #qithub_dev\n" . $arg['status'];
    $toot_msg  = rawurlencode($toot_msg);

    $visibility   = $arg['visibility'];
    $access_token = $arg['access_token'];
    $domain       = $arg['domain'];

    $query  = 'curl -X POST';
    $query .= " -d 'status=${toot_msg}'";
    $query .= " -d 'visibility=${visibility}'";
    $query .= " --header 'Authorization: Bearer ${access_token}'";
    $query .= " -sS https://${domain}/api/v1/statuses;";
    
    $result = 'OK';
    $result_value = `$query`;

    echo encode_array_to_api([
        'result' => $result,
        'value'  => $result_value,
    ]);

// 必須項目が足りない場合の処理
} else {

    $temp = print_r($arg, true);

    die(encode_array_to_api([
        'result' => 'NG',
        'value'  => "必須項目が足りません。\n${temp}",
    ]));
}


die();
/* ---------------------------------------------------------------------
    Functions
   --------------------------------------------------------------------- */
/**
 *  トゥートに最低限必要な項目を網羅しているかチェックします
 *
 * @param  array   $arg   API から渡された配列
 * @return boolean        必須項目を満たしていた場合は true
 */
function is_requirement_complied($arg)
{
    return isset($arg['domain']) && isset($arg['access_token']) && isset($arg['status']) && isset($arg['visibility']);
}


/* ---------------------------------------------------------------------
    Getter Functions
   --------------------------------------------------------------------- */
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
