<?php error_reporting(E_ALL);

include_once("constants.php.inc");
include_once('functions.php.inc');

// 日本語 UTF-8 タイムゾーンを東京にセット
set_env_utf8_ja();

$saved = [
    'one'   => 1,
    'two'   => 2,
    'three' => 3,
];

$new = [
    'one'   => 1,
    'three' => 3,
    'four'  => 4,
    'six'   => 6,
];

//array_unshift($saved,$new);
//$saved = $saved + $new;
$diff = array_diff_key($new, $saved);
$saved = $diff + $saved;

print_r($saved);

die;

// 人気のQiita記事を取得
$feed_url   = 'https://qiita.com/popular-items/feed';
$feed_xml   = simplexml_load_file($feed_url);
$feed_json  = json_encode($feed_xml);
$feed_array = json_decode($feed_json,TRUE);

foreach ($feed_array[entry] as $item) {
    $x              = array();
    $x['id_raw']    = (string)  $item[id];
    $x['id']        = (integer) get_num_fron_id($x[id_raw]);
    $x['title']     = (string)  $item[title];
    $x['link']      = (string)  $item[link]['@attributes'][href];
    $x['author']    = (string)  $item[author][name];
    $x['published'] = (string)  $item[published];
    $x['updated']   = (string)  $item[updated];

    $data[] = $x;
}

$result = [
    'result' => 'OK',
    'value' => $data
];

$json_raw = json_encode($result);
$json_enc = urlencode($json_raw);

echo $json_enc;
