<?php error_reporting(E_ALL);

include_once("constants.php.inc");
include_once('functions.php.inc');

// 標準入力を取得
$arg = get_api_input_as_array();

// デバッグモード取得
$is_mode_debug = ( isset($arg['mode']) && 'debug' == $arg['mode'] );

// 保存データのID（読み込みキー）
$id_data = 'id_saved_data';

// 保存済みデータの読み込み
$data_old = load($id_data);
if ($data_old == IS_DATA_UNINITIALIZED) {
    $data_old = array();
}

// 人気のQiita記事を取得
$feed_url   = 'https://qiita.com/popular-items/feed';
$feed_xml   = simplexml_load_file($feed_url);
$feed_json  = json_encode($feed_xml);
$feed_array = json_decode($feed_json, true);

foreach ($feed_array[entry] as $item) {
    $x              = array();
    $x['id_raw']    = (string)  $item[id];
    $x['id']        = (integer) get_num_fron_id($x[id_raw]);
    $x['title']     = (string)  $item[title];
    $x['link']      = (string)  $item[link]['@attributes'][href];
    $x['author']    = (string)  $item[author][name];
    $x['published'] = (string)  $item[published];
    $x['updated']   = (string)  $item[updated];

    $data_new[$x['id']] = $x;
}

$data_diff = array_diff_key($data_new, $data_old);
$data_new  = $data_diff + $data_old;

// デバッグモード（"&mode=debug"）の場合は保存しない
$is_saved_success = ($is_mode_debug) ?: save($id_data, $data_new);

if ($is_saved_success) {
    $result = [
        'result' => 'OK',
        'value' => [
            'diff'  => $data_diff,
            'top20' => $data_new,
        ],
    ];
} else {
    $result = [
        'result' => 'NG',
        'value' => 'Data save fail.',
    ];
}


$json_raw = json_encode($result);
$json_enc = urlencode($json_raw);

echo $json_enc;
