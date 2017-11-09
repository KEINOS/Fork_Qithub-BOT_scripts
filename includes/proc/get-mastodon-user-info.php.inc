<?php
// プロセス get-mastodon-user-info
// ---------------------------------------------------------------------
// マストドンのユーザーアカウントおよびフォロワーの情報を表示する
//
// 一度取得した情報はキャッシュされる。リクエスト・パラメータ'use_cash' 
// に `false` が指定されていた場合は新規取得（更新）される。
// パラメーター 'id' にユーザーID が指定されていた場合は、そのユーザーお
// よびフォロワーの情報が表示される。
// また、'id', 'use_cash', 'mode'のオプション・パラメータは本体スクリプ
// トのURLクエリと連動しており以下のように受け取ることができる。
//
//   BOT のアカウントを取得する例（JSON形式）
//       /qithub/?process=get-mastodon-user-info
//   BOT のアカウントを取得する例（デバッグモード,配列表示）
//       /qithub/?process=get-mastodon-user-info&mode=debug
//   指定したユーザーのアカウントを取得する例
//       /qithub/?process=get-mastodon-user-info&id=3835

// Mastodon API に必要なキーの取得
$keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');

// 基本のパラメーター設定
$params = [
    'domain'       => $keys_api['domain'],
    'access_token' => ACCESS_TOKEN_MASTODON,
];

// オプション・パラメーターの追加
if (isset($_GET['id']) && is_numeric($_GET['id'])) {
    $params   = $params + ['id' => (int) $_GET['id']];
}
if (isset($_GET['use_cash']) && ! empty($_GET['use_cash'])) {
    $use_cash = ('false' !== strtolower($_GET['use_cash']));
    $params   = $params + ['use_cash' => $use_cash];
}

// マストドンのユーザー＆フォロワー情報取得（API経由）
$result_api = run_script('system/get-mastodon-user-info', $params, false);
$result     = decode_api_to_array($result_api);

// リクエスト結果の表示
if (isset($result['result']) && 'OK' == $result['result']) {
    if (IS_MODE_DEBUG) {
        // 配列形式で出力（デバッグ確認用）
        echo 'OK' . BR_EOL;
        echo_on_debug(json_decode($result['value'], JSON_OBJECT_AS_ARRAY));
    } else {
        // JSON形式で出力
        echo $result['value'];
    }
} else {
    echo 'Request error' . BR_EOL;
    echo_on_debug(json_decode($result['value'], JSON_OBJECT_AS_ARRAY));
}

