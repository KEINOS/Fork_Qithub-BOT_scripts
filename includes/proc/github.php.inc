<?php

// プロセス 'github'
// ---------------------------------------------------------------------
// GitHub からの WebHook 処理
// クエリの'method'オプションが指定されていない場合は受け取ったデータを
// 保存。'method'オプションによって保存データの閲覧・削除を行うデータを
// 保存するキー（データID）

$id_data = 'WebHook-GitHub';

// 保存済みデータの読み込み
$log_data = load_data($id_data);
if ($log_data === LOAD_DATA_EMPTY) {
    $log_data = array();
}

// ログの操作（view, delete）
// 'method'オプションによる保存データの扱い
if (isset($_GET['method'])) {
    switch ($_GET['method']) {
        // ログの一部削除
        // ログのキー（=タイムスタンプ）を指定して削除
        case 'delete':
            $key_to_delete = $_GET['key'];
            // 削除の実行（データのアップデート）
            if (isset($log_data[$key_to_delete])) {
                unset($log_data[$key_to_delete]);
                if (save_data($id_data, $log_data)) {
                    $log_data = load_data($id_data);
                } else {
                    $log_data = 'Error on updating log';
                }

                echo '<pre style=\'width:100%;overflow: auto;white-space: pre-wrap; word-wrap: break-word;\'>' . PHP_EOL;
                print_r($log_data);
                echo '</pre>' . PHP_EOL;
            } else {
                echo "削除するデータのキーが指定されていません。";
            }
            break;

        // ログ表示（WebHook からのデータの保存内容の確認）
        case 'view':
            echo '<pre style=\'width:100%;overflow: auto;white-space: pre-wrap; word-wrap: break-word;\'>' . PHP_EOL;
            print_r($log_data);
            echo '</pre>' . PHP_EOL;
            break;
        default:
            break;
    }
    die();

// WebHook からの受け取りデータの保存
} else {
    // データの保存キー
    $timestamp = date("Ymd-His");
    // データの作成
    $log_data[$timestamp] = [
        'getallheaders' => getallheaders(),
        'get'           => $_GET,
        'post'          => $_POST,
        'ip'            => $_SERVER["REMOTE_ADDR"],
        'host'          => gethostbyaddr($_SERVER["REMOTE_ADDR"]),
        'raw_post_data' => file_get_contents('php://input'),
    ];
    // 保存実行
    $result = save_data($id_data, $log_data);
    // レスポンス
    if ($result==true) {
        echo "Data saved." . PHP_EOL;
        echo "Data ID/key was: ${id_data}/${timestamp}" . PHP_EOL;
    }
}
