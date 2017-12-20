<?php
/* =====================================================================
    deploy-item-to-qiita
    
    指定された Qiita 記事 ID の GitHub (Qithub/items/）上のコンテンツを
    Qiita にデプロイ（更新）します。（作成済みの記事に限る）
    
    仕様
        - Qiita 記事のステータスは「限定公開」のままです。トゥートによる
          「お気に入り」数によって公開される機能が実装される予定。
        - 開発環境（is_env_dev = 1）の場合
            - 自動的に「test」タグが挿入されます
            - 更新先の Qiita 記事は固定です

    必須パラメーター
        - "id" : string : Qiita記事ID（Qithubアカウントの記事に限る）
        - "access_token" : string : Qiita API のアクセストークン

    オプションパラメーター
        - "is_env_dev"     : bool : 0 = 本番環境, 1 = 開発環境
        - "is_mode_debug"  : bool : 0 = 通常動作, 1 = デバッグモード
        - "max_tag_number" : int  : 記事のタグ数（デフォルト&最大値は5）

    RESTful パラメーター
        - "id"   : Qiita 記事 ID
        - "mode" : "debug" の場合はデバッグ・モードで動作
        - サンプル
            - ポスト・クエリ
              https://blog.keinos.com/qithub_dev/?process=deploy-item-to-qiita&id=7157c17765e328917667&mode=debug
            - ポスト結果の確認先
              https://qiita.com/KEINOS_BOT/private/58ca66a808aa774c8233
   ================================================================== */

// SideCI要件により外部ファイル化
require('constants.php.inc');
require('functions.php.inc');

// 日本語＆タイムゾーンを東京に環境設定をセット
set_utf8_ja();

// CLIからの標準入力を取得
$args          = get_api_input_as_array();
$is_env_dev    = (! empty($args['is_env_dev'])    ) ? true : false;
$is_mode_debug = (! empty($args['is_mode_debug']) ) ? true : false;

// DEVELOP 環境か否（ DEPLOY 環境）かのセット
set_env_as_dev($is_env_dev);

// パラメーターの設定
if (empty($args['max_tag_number'])) {
    $max_tag_number = MAX_TAG_NUM_QIITA;
} else {
    $max_tag_number = $args['max_tag_number'];
    if(MAX_TAG_NUM_QIITA < $max_tag_number){
        $max_tag_number = MAX_TAG_NUM_QIITA;
    }
}

if (empty($args['access_token'])) {
    $status   = 'NG';
    $contents = 'Error: Access token is empty';
    print_output_as_api($status, $contents);
}

if (empty($args['id'])) {
    if ($is_env_dev) {
        $args['id'] = ID_ITEM_GITHUB_DUMMY;
    } else {
        $status   = 'NG';
        $contents = 'Error: Qiita item ID is empty';
        print_output_as_api($status, $contents);
    }
}

$access_token   = $args['access_token'];
$id_item_qiita  = ($is_env_dev) ? ID_ITEM_QIITA_DUMMY : $args['id'];
$id_item_github = $args['id'];

// GitHub 側のコンテンツの取得
$url_github_raw = create_url_github_raw($id_item_github);
$id_uniq = uniqid();
$id_now  = get_id_timestamp();
$content = get_item_github_raw($url_github_raw . "?${id_uniq}");
$content = sanitize_content($content);
if (false == $content) {
    print_output_as_api('NG', 'Error: Can not get contents');
}

// Qiita 側に POST（PATCH）するためにフォーマット
// タイトル、タグ、本文の抜き出しとサニタイズ
$info_document = get_lines_first_and_second($content);

$title   = trim($info_document[0], '#');
$title   = ($is_env_dev) ? "${title} ${id_now}" : $title;

$tags    = get_tags_as_array(trim($info_document[1], '[]'));
$tags    = format_tags_for_qiita($tags,$max_tag_number);

$body    = get_lines_rest($content);

// Qiita API へポストするデータの準備
// https://qiita.com/api/v2/docs#patch-apiv2itemsitem_id
$data_array = [
    'body'      => $body,  //Markdown形式の本文
    'coediting' => false,  //共同更新状態 (Qiitaではfalseのみ)
    'private'   => true,   //限定共有状態かどうかを表すフラグ
    'tags'      => $tags,  //投稿に付けるタグ
    'title'     => $title, //投稿のタイトル
];
$data_json = json_encode($data_array);

// 新規投稿をする場合のサンプル（ここでは利用しない）
// $result = post_to_qiita($data_json, $access_token);

// Qiita API へ記事を投稿（記事の更新）
$result = patch_to_qiita($id_item_qiita, $data_json, $access_token);

// 処理結果を表示
print_output_as_api('OK', $result);

die();
