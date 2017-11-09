<?php
// プロセス 'toot-daily'
// ---------------------------------------------------------------------
// 日付ごとのスレッドでトゥートするサンプル
//
// 定例処理用のプロトタイプ。トゥートした日付の初トゥートの場合は普通に
// トゥート（親トゥート）し、以降の同日トゥートは返信でトゥート（子トゥー
// ト）します。子トゥートは親に対しての返信でなく、１つ前のトゥートに対
// して返信される。（トゥートクリック時にスレッド内容がわかるように）

// トゥートに必要なAPIの取得
$keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');

// トゥートのIDと日付を保存するキー（データID）
$id_data = 'toot_id_and_date_of_daily_toot';

// トゥート済みのトゥートIDと日付を取得
$info_toot = load_data($id_data);

// 今日の日付を取得
$id_date = (int) date('Ymd');

// トゥートIDの初期化
$id_toot_current  = ''; // １つ前のトゥートID
$id_toot_original = ''; // 親のトゥートID

// 保存データの有無確認
if ($info_toot !== LOAD_DATA_EMPTY) {
    // 本日の初トゥートフラグ（保存日の比較）
    $is_new_toot = ($info_toot['id_date'] !== $id_date);
    // トゥートIDの取得
    $id_toot_current  = $info_toot['id_toot_current'];
    $id_toot_original = $info_toot['id_toot_original'];
} else {
    // 本日の初トゥートフラグ
    $is_new_toot = true;
}

// 今日の初トゥート実行とID＆日付の保存
if ($is_new_toot) {
    // トゥート内容の作成
    $date_today = date('Y/m/d');
    $msg = "${date_today} のトゥートを始めるよ！";

    // トゥートのパラメータ設定（新規投稿）
    $params = [
        'status'       => $msg,
        'domain'       => $keys_api['domain'],
        'access_token' => ACCESS_TOKEN_MASTODON,
        'visibility'   => 'unlisted',
    ];

// 本日のトゥート発信済みなので、それに返信
} else {
    // タイムスタンプ
    $timestamp = time();
    // タイムスタンプの偶数・奇数でメッセージを変更
    // 後日実装予定の新着Qiita記事がフォロワーの場合に備えて
    // の準備として
    if ($timestamp % 2 == 0) {
        $msg_branch = "偶数だにゃーん\n\n";
    } else {
        $msg_branch = "奇数だにゃーん\n\n";
    }
    // トゥート内容の作成
    $date_today = date('Y/m/d H:i:s', $timestamp);
    $msg  = $msg_branch;
    $msg .= "Posted at :${date_today}\n";
    $msg .= "In reply to :${id_toot_current}\n";

    // トゥートのパラメーター設定（返信投稿）
    $params = [
        'status'         => $msg,
        'domain'         => $keys_api['domain'],
        'access_token'   => ACCESS_TOKEN_MASTODON,
        'in_reply_to_id' => $id_toot_current,
        'visibility'     => 'unlisted',
    ];
}
// トゥートの実行
$result_toot = post_toot($params);

// トゥート結果の表示とトゥートID＆今日の日付を保存
if ($result_toot == TOOT_SUCCESS) {
    // トゥートIDの取得
    $id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
    // 親トゥートのID取得
    $id_toot_original = ($is_new_toot) ? $id_toot_current : $id_toot_original;
    // 保存するデータ
    $info_toot_to_save = [
        'id_toot_current'  => $id_toot_current,
        'id_toot_original' => $id_toot_original,
        'id_date'          => $id_date,
    ];
    // 今回のトゥートIDの保存（返信の場合はデイジーチェーン）
    /** @todo デイジーチェーンの場合、途中でトゥートがスパム
     *        Qiita記事だったなどで削除された場合にチェーン
     *        が切れてしまう。チェックしてからトゥート？
     */
    $result_save = save_data($id_data, $info_toot_to_save);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo "Toot info saved." . BR_EOL;
    }
    echo ($is_new_toot) ? "New toot " : "Reply toot ";
    echo " posted successfuly." . BR_EOL;
    print_r($info_toot_to_save);
} else {
    echo "Toot fail." . BR_EOL;
}

