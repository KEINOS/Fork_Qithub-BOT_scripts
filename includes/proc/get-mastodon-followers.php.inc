<?php
// プロセス get-mastodon-followers
// ---------------------------------------------------------------------
// マストドンのフォロワーの情報を表示する
// "get-mastodon-user-info.php.inc" のフォロワー情報取得に特化したもの。
//

// Mastodon API に必要なキーの取得
$keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');

// 基本のパラメーター設定
$params = [
    'domain'       => $keys_api['domain'],
    'access_token' => ACCESS_TOKEN_MASTODON,
    'use_cash'     => false,
];

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

