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

// CLIからの標準入力を取得
$arg = $argv;

// 引数１は必須
if (empty($arg[1])) {
    print_r($arg);
    die("Argument is empty.");
}

// 引数のデコードとAPIキー（アクセストークンなど）を取得
$arg = urldecode(trim($arg[1]));
$arg = json_decode($arg, JSON_OBJECT_AS_ARRAY);


$is_mode_debug = $arg['is_mode_debug'] ?: false;
$keys_api      = $arg['keys'];
$_GET          = $arg['get'];
$_POST         = $arg['post'];

// トゥート操作に必要な基本情報の設定
$domain       = $keys_api['domain'];
$access_token = $keys_api['access_token'];

// 前回トゥートがあった場合は「削除」
$name_file_tootID_last = 'last_toot.dat';
$path_dir_tootID_last  = dirname(__FILE__) . DIRECTORY_SEPARATOR;
$path_file_tootID_last = $path_dir_tootID_last . $name_file_tootID_last;
$toot_msg_extra        = '';

if (file_exists($path_file_tootID_last)) {
    $tootID_last_enc = file_get_contents($path_file_tootID_last);
    $tootID_last_dec = unserialize($tootID_last_enc);

    // 削除用のクエリ作成
    $query = get_query_delete_toot($tootID_last_dec, $access_token, $domain);

    // 削除の実行
    $result_json  = trim(`$query`); // exec($query)
    $result_array = json_decode($result_json, JSON_OBJECT_AS_ARRAY);

    // デバッグ時の出力確認
    if( $is_mode_debug ){
        print_r($query);
        print_r($result_array);        
    }

    // 削除した旨の追加メッセージ
    $toot_msg_extra  = "\nThe last toot ID is : ${tootID_last_dec}";
}

// トゥート内容の作成
$time_stamp = date("Y/m/d H:i:s");
$toot_msg   = "#test \n";
$toot_msg  .= "Hello World! ${time_stamp} ${toot_msg_extra}\n";
$toot_msg  .= "#qithub_dev ";

//メッセージをcURL用にエスケープ
$toot_msg = rawurlencode($toot_msg);

// トゥートの実行 (request API)と結果の取得
$visibility   = 'unlisted'; //投稿のプライバシー設定→「未収載」
$query        = get_query_toot($toot_msg, $visibility, $access_token, $domain);
$result_json  = trim(`$query`); // exec($query)
$result_array = json_decode($result_json, JSON_OBJECT_AS_ARRAY);

// トゥートIDの保存
$tootID_last_raw = $result_array['id'];
$tootID_last_enc = serialize($tootID_last_raw);

// デバッグ時の保存されるトゥートの確認
if( $is_mode_debug ){
    echo "Last raw ID is: '${tootID_last_raw}'\n";
    echo "Last enc ID is: '${tootID_last_enc}'\n";
    echo "Save to: ${path_file_tootID_last}\n";    
}

file_put_contents($path_file_tootID_last, $tootID_last_enc);

// トゥート結果を表示
if ($is_mode_debug) {
    print_r($result_array);
} else {
    echo $result_json;
}

die();

/* ---------------------------------------------------------------------
    Save and load data Functions
   ------------------------------------------------------------------ */


/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */
function get_query_delete_toot($id_toot, $access_token, $domain)
{

    $query  = 'curl -X DELETE';
    $query .= " --header 'Authorization: Bearer ${access_token}'";
    $query .= " -sS https://${domain}/api/v1/statuses/${id_toot};";

    return $query;
}

function get_query_toot($toot_msg, $visibility, $access_token, $domain, $id_toot_reply = null)
{

    if ($id_reply === null) {
        $query  = 'curl -X POST';
        $query .= " -d 'status=${toot_msg}'";
        $query .= " -d 'visibility=${visibility}'";
        $query .= " --header 'Authorization: Bearer ${access_token}'";
        $query .= " -sS https://${domain}/api/v1/statuses;";
    } else {
        $query  = "curl -X POST";
        $query .= " -d 'status=${toot_msg}'";
        $query .= " -d 'visibility=${visibility}'";
        $query .= " -d 'in_reply_to_id=${id_toot_reply}'";
        $query .= " --header 'Authorization: Bearer ${access_token}'";
        $query .= " -sS https://${host}/api/v1/statuses;";
    }

    return $query;
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
