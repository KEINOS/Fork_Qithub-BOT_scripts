<?php
// プロセス 'say-hello-world'
// ---------------------------------------------------------------------
// 'plugins/say-hello-world' を利用したサンプル
//
// 'system/data-io','system/delete-toot','system/post-toot'の
// ラッパー関数を使って、前回トゥートしたトゥートを削除し、
// 'say-hello-world'プラグインより取得したメッセージを新規トゥー
// トする。

// トゥートIDの保存キー（データID）
$id_data = 'last-toot-id_say-hello-world';

// 前回トゥートのIDを取得
$id_pre_toot  = load_data($id_data);
$has_pre_toot = ($id_pre_toot !== LOAD_DATA_EMPTY);

// トゥートに必要なAPIの取得
$keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon_dev');

// 前回トゥートを削除
$msg_toot_deleted = '';
$is_toot_deleted  = false;
if ($has_pre_toot) {
    $is_toot_deleted = delete_toot([
        'domain'       => $keys_api['domain'],
        'access_token' => ACCESS_TOKEN_MASTODON,
        'id'           => $id_pre_toot,
    ]);
    $msg_toot_deleted = ( $is_toot_deleted ) ? "Last toot has been deleted.\n" : "Error deleting toot.\n";
}

// トゥートメッセージの作成
$timestamp       = date("Y/m/d H:i:s");
$msg_toot        = "\n" . "Tooted at: ${timestamp}";
$msg_last_tootid = ( $has_pre_toot ) ? "Last toot ID was: ${id_pre_toot}\n" : '';
$msg_toot       .= "\n" . $msg_last_tootid . $msg_toot_deleted;
$params = [
    'say_also' => $msg_toot,
];
$result_api = run_script('plugins/say-hello-world', $params, false);
$result     = decode_api_to_array($result_api);

// トゥートの実行
if ($result['result'] == 'OK') {
    $result_toot = post_toot([
        'status'       => $result['value'],
        'domain'       => $keys_api['domain'],
        'access_token' => ACCESS_TOKEN_MASTODON,
        'visibility'   => 'unlisted',
    ]);

    if ($result_toot) {
        $id_pre_toot = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
        // 今回のトゥートIDの保存
        $result = save_data($id_data, $id_pre_toot);
        if ($result['result'] == true) {
            echo "Saved last toot ID as : ${id_pre_toot}" . PHP_EOL;
            echo "Tooted msg was ${msg_toot}" . PHP_EOL;
        }
    }
}
