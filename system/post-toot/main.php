<?php
/**
 * 新規トゥートを行います
 *
 *
 *
 * 【このスクリプトの Qithub API パラメーター】
 *
 * ■受け取り必須項目
 *   必須項目
 *     'domain'       => string トゥート先のドメイン名(ホスト名のみ)
 *     'access_token' => string トゥートに必要なアクセストークン
 *     'status'       => string トゥート内容
 *     'visibility'   => string 'public','unlisted','private','direct'
 *   オプション項目
 *     'spoiler_text'   => string この項目があると'CW'モードになり'status'
 *                                は「もっと読む」内の非表示領域になります。
 *     'in_reply_to_id' => string この項目があると値のトゥートIDに対して
 *                                返信扱いになります。
 *     'is_mode_dev'    => string 'yes' = DEV環境用の動作
 *     'is_mode_debug'  => bool   デプロイ、DEV環境関係なくデバッグモード
 *                                で動かしたい場合は true
 *
 * ■出力項目
 *     'result' => string 'OK','NG'
 *     'value'  => json    MastodonからのレスポンスをJSONのまま返します。
 *
 */

/* =====================================================================
    Main
   ===================================================================== */
// 関数の読み込み（Side CI 対応）
require('functions.php.inc');

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 標準入力を取得
$arg = get_api_input_as_array();

// ■ 必須項目を満たしていた場合
// Mastodon API 用のリクエスト・クエリを作成し、トゥート後、Mastodon API
// からのレスポンス結果（JSON）をそのまま、 Qithub API の "Value" に代入
// して、Qithub API フォーマットで返します。
// ■ 必須項目をみたしていない場合
// "result" = "NG" で Qithub API フォーマットで返します。
if (is_requirement_complied($arg)) {
    // DEV環境やデバッグモードの場合はトゥートにそれがわかるように付加
    $tag_additional = '';
    if (isset($arg['is_mode_dev']) && ('yes' == $arg['is_mode_dev'])) {
        $tag_additional .= "\n#test #qithub_dev";
    }
    if (isset($arg['is_mode_debug']) && ('yes' == $arg['is_mode_debug'])) {
        $tag_additional .= " [DEBUG MODE]";
    }

    // トゥートメッセージの作成＆API用にエスケープ
    $toot_msg  = $arg['status'] . $tag_additional;
    $toot_msg  = rawurlencode($toot_msg);

    // トゥートに必須な情報を設定
    $visibility   = $arg['visibility'];
    $access_token = $arg['access_token'];
    $domain       = $arg['domain'];

    // 「in_reply_to_id」がある場合は返信用のクエリを付加
    $query_additional = '';
    if (isset($arg['in_reply_to_id']) && !empty($arg['in_reply_to_id'])) {
        $in_reply_to_id    = $arg['in_reply_to_id'];
        $query_additional .= " -d 'in_reply_to_id=${in_reply_to_id}'";
    }

    // 「spoiler_text」の指定がある場合は「CW」でトゥートのクエリを付加
    if (isset($arg['spoiler_text']) && !empty($arg['spoiler_text'])) {
        $spoiler_text = $arg['spoiler_text'];
        $spoiler_text = rawurlencode($spoiler_text);
        $query_additional .= " -d 'spoiler_text=${spoiler_text}'";
    }

    // Mastodon API 用のトゥート・リクエストのクエリ作成
    $query  = 'curl -X POST';
    $query .= " -d 'status=${toot_msg}'";
    $query .= " -d 'visibility=${visibility}'";
    $query .= $query_additional;
    $query .= " --header 'Authorization: Bearer ${access_token}'";
    $query .= " -sS https://${domain}/api/v1/statuses;";

    // トゥートの実行
    $result_value = `$query`;

    /** @todo サーバが500の場合なども'OK'を返してしまうので要改善 */
    $result = 'OK';

    // 実行結果を Qithub API 互換で返す
    echo encode_array_to_api([
        'result' => $result,
        'value'  => $result_value,
    ]);
} else {
    $temp = print_r($arg, true);

    die(encode_array_to_api([
        'result' => 'NG',
        'value'  => "必須項目が足りません。\n${temp}",
    ]));
}


die();
