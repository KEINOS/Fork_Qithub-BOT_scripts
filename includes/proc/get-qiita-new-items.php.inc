<?php
// 'get-qiita-new-items'
// ---------------------------------------------------------------------
// Qiita記事の新着N件を表示するサンプル
//
// クエリのパラメーター'max_items'が指定されている場合はその件数
// ぶん、未指定の場合はデフォルトの5件が表示される。専用のラッパー
// を作らず汎用的な呼び出し方法で 'system/get-qiita-new-items'を
// 利用するサンプル

if (isset($_GET['max_items'])) {
    $max_items = (int) $_GET['max_items'];
} else {
    $max_items = 5;
}

$params = [
    'max_items' => $max_items,
];

$result_api = run_script('system/get-qiita-new-items', $params, false);
$result     = decode_api_to_array($result_api);

if ($result['result']) {
    echo '<pre>';
    print_r($result);
    echo '</pre>';
}
