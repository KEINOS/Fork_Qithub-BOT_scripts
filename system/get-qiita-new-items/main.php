<?php
/**
 * 新着 Qiita 記事を取得します
 *
 *
 * 【このスクリプトの Qithub API パラメーター】
 *
 * ■受け取り必須項目
 *     なし
 *
 * ■出力項目
 *     'max_items' => integer Qiitaから取得する件数。（最大20件）
 *
 * @todo：前回取得の差分のみを返す
 */

/* =====================================================================
    Main
   ===================================================================== */

// 日本語環境＆タイムゾーンを東京に設定
set_utf8_ja();

// 標準入力を取得
$arg = get_api_input_as_array();

// 新着記事の取得件数
if( isset($arg['max_items']) && $arg['max_items'] <= 20 ){
    $max_items = (int) $arg['max_items'];
} else {
    $max_items = 10;
}

// Qiita API の URL
$url_qiita_new_items = "https://qiita.com/api/v2/items?page=1&per_page=${max_items}";

// User Agent 設定
$ctx = stream_context_create([
    'http' => [
        'method' => 'GET',
        'header' => 'User-Agent: BOT Agent by @Qithub@Qiitadon',       
    ]
]);

// エラー時のHTTPステータスコードを取得
if( ( $result_json = @file_get_contents($url_qiita_new_items,false,$ctx) ) == FALSE ){
    list($version,$status_code,$msg) = explode(' ',$http_response_header[0], 3);
    echo encode_array_to_api([
        'result' => 'NG',
        'value'  => "Error:${status_code}",
    ]);

    die();

// 正常にfile_get_contentsで取得できた場合の処理
} else {
    $result = json_decode($result_json, JSON_OBJECT_AS_ARRAY);
    
    $items = array();
    foreach($result as $item){
        $id = $item['id'];
        $items[$id] = [
            'title' => $item['title'],
            'tags'  => $item['tags'],
            'url'   => $item['url'],
            'user'  => $item['user'],
        ];
    }

    echo encode_array_to_api([
        'result' => 'OK',
        'value'  => $items,
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
