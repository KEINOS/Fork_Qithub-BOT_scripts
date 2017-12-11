<?php
// =====================================================================
// プロセス 'toot-daily-qiita-items'
// =====================================================================
// 定時ごと（１時間おき）のスレッドに新着 Qiita 記事をトゥートしていく。
//
// ■定時の初回トゥートの場合
// 1) 前スレッドでトゥートされたタグ一覧を記載した「取りまとめ」をトゥート
// 2) 新しいスレッドを開始する旨の「宣言トゥート」をトゥート
//    ※いずれも「公開」でトゥート（DEV環境の場合は「未収載」）
//
// ■定時内のトゥートの場合
// 1) 前のトゥートに返信で新着Qiita記事をトゥートしてスレッドにしていく
//   - Qiita記事が Qiitadon ユーザーの記事の場合は「未収載」でトゥート
//   - それ以外は「非公開」（フォロワーのみ閲覧可）でトゥート
//   - 「非公開」トゥートは Boostoot できるようにトゥート用リンクが付加
//
// DEV環境の動作確認用リンク
// https://blog.keinos.com/qithub_dev/?process=toot-daily-qiita-items&mode=debug

// 初期化
$result_toot      = TOOT_FAIL;
$is_new_toot      = false;
$id_toot_current  = ''; // １つ前のトゥートID
$id_toot_original = ''; // 親のトゥートID
$max_items        = 10; // 取得するQiita記事の数

// 取得：トゥートに必要なAPI
$keys_api     = get_api_keys('../../qithub.conf.json', 'qiitadon');
$domain       = $keys_api['domain'];
$access_token = ACCESS_TOKEN_MASTODON;

// 読み込み：最後のトゥートIDと現在のスレッドの親トゥートID
$id_data_toot = 'toot_id_and_date_of_daily_toot';
$info_toot    = load_data($id_data_toot);

// 読み込み：新着Qiita記事ID一覧（トゥート済み）
$id_data_qiita   = 'qiita_id_tooted';
$qiita_items_old = load_data($id_data_qiita);
$qiita_items_old = ( LOAD_DATA_EMPTY == $qiita_items_old ) ? array() : $qiita_items_old;

// 読み込み：タグ一覧（トゥート済み）
$id_data_tags     = 'tag_list_of_daily_toot';
$list_tags_tooted = load_data($id_data_tags);
$list_tags_tooted = ( LOAD_DATA_EMPTY == $list_tags_tooted ) ? array() : $list_tags_tooted;

// 削除：動作確認用の要素の削除
//unset($qiita_items_old['9412acb2a7ff37ff9ad0']);

// 比較・差分取得：最新の新着Qiita記事の取得とトゥート済み記事との比較
$result_api = run_script(
    'system/get-qiita-new-items',
    ['max_items' => $max_items],
    RUN_FOREGROUND
);
$qiita_items_new  = decode_api_to_array($result_api)['value'];
$qiita_items_diff = array_diff_key($qiita_items_new, $qiita_items_old);

// 取得：現在の時間（YmdH）をスレッドのIDとして取得（１時間毎のスレッド）
$id_thread = date('Y/m/d H:00');
echo_on_debug($id_thread, 'Current Thread ID');

// フラグ設定：宣言トゥートが発動済みか
// 発動済みの場合：親トゥートと最新（１つ前の）トゥートをセット
if (LOAD_DATA_EMPTY !== $info_toot) {
    // トゥートIDの取得
    $id_toot_original = trim($info_toot['id_toot_original']);
    $id_toot_current  = trim($info_toot['id_toot_current']);

    // 宣言トゥート済みフラグを立てる
    if ($info_toot['id_thread'] !== $id_thread) {
        $is_new_toot = true;
    } elseif (! empty($id_toot_original) && ! empty($id_toot_current)) {
        $is_new_toot = false;
    } elseif (empty($id_toot_original) && empty($id_toot_current)) {
        $is_new_toot = true;
    } elseif (! empty($id_toot_original) && empty($id_toot_current)) {
        $id_toot_current = $id_toot_original;
        $is_new_toot = false;
    }
} else {
    $is_new_toot = true;
    echo_on_debug('新規トゥートのフラグをセットしました');
}

// ---------------------------------------------------------------------
//  「取りまとめ」トゥート と 宣言（スレッドの親)トゥート
// ---------------------------------------------------------------------
// とりまとめ（前回スレッド内のハッシュタグ一覧）をトゥートしたのち、次
// の新着Qiita記事トゥートのスレッドを開始する宣言トゥート（＝スレッドの
// 親トゥート）を行う。
//
if ($is_new_toot) {
    // -----------------------------------------------------------------
    //   宣言前の「取りまとめ」トゥート
    // -----------------------------------------------------------------
    if (! empty($list_tags_tooted) && ! empty($id_toot_original)) {
        // スレッドの最終トゥートID
        $id_thread_last = $info_toot['id_thread'];

        // 取得：取りまとめ時刻
        $thread_date  = date("Y/m/d", strtotime($id_thread_last));
        $thread_hour  = date("H 時", strtotime($id_thread_last));

        // CW文
        // -------------------------------------------------------------
        $spoiler_text =<<<EOD
📋【​:qiita:​のタグ一覧】(${thread_date} ${thread_hour})
EOD;
        // ヘッダー文
        // -------------------------------------------------------------
        $msg_header =<<<EOD
以下のタグの記事がこの１時間に :qiita: に投稿されました。
➖➖➖➖➖➖➖➖➖➖
EOD;
        // フッター文
        // -------------------------------------------------------------
        $msg_footer =<<<EOD
➖➖➖➖➖➖➖➖➖➖
これらのタグを含む記事へのリンクは、このトゥートのスレッドに「非公開」もしくは「未収載」でトゥートされています。
EOD;

        // ペイン（CW、ヘッダー、フッター）の文字数をカウント
        $count_char_pane  = mb_strlen($spoiler_text); // CW文のぶん
        $count_char_pane += mb_strlen($msg_header);   // ヘッダー文のぶん
        $count_char_pane += mb_strlen($msg_footer);   // フッター文のぶん
        $count_char_pane += mb_strlen("\n\n");        // タグ一覧の前後の改行ぶん
        $count_char_pane += mb_strlen('[nn/mm]');     // 取りまとめのページ分割表示ぶん

        // 設定：トゥート済みタグ一覧の抽出
        // 500文字（MAX_TOOT_CHAR）区切りで $msg_tags 配列にセット
        $msg_tags = array();
        $tmp_tags = ''; // 文字数オーバー時のロールバック用
        ksort($list_tags_tooted); // タグ一覧を昇順でソート
        foreach ($list_tags_tooted as $tag) {
            // 空白タグはスキップ
            if( empty( trim( $tag )){
                continue;
            }
            $tmp_tag          = "#${tag}\n";
            $count_char_tag   = mb_strlen($tmp_tag);
            $count_char_tags  = mb_strlen($tmp_tags);
            $count_char_tags  = $count_char_tag  + $count_char_tags;
            $count_char_total = $count_char_tags + $count_char_pane;

            if (MAX_TOOT_CHAR <= $count_char_total) {
                // MAX CHAR数を超えたらオーバーぶんをロールバックして配列にセット
                $msg_tags[] = trim($tmp_tags);
                $tmp_tags   = '';
                $tmp_tags   = $tmp_tag;
            } else {
                $tmp_tags  .= $tmp_tag;
            }
        }
        // 設定：残りのタグをセット
        if (! empty($tmp_tags)) {
            $msg_tags[] = trim($tmp_tags);
        }

        $denominator    = count($msg_tags); //分母
        $is_tag_splited = ( 1 < $denominator );

        // 500文字ごとに取りまとめをトゥート
        foreach ($msg_tags as $fraction => $tag_set) {
            echo_on_debug($msg_tags);

            if ($is_tag_splited) {
                $fraction_tmp = $fraction +1;
                $msg_additional_fraction = "[${fraction_tmp}/${denominator}]";
            } else {
                $msg_additional_fraction = '';
            }
            //
            // とりまとめトゥートのメッセージ内容
            //----------------------------------------------------------
            $msg =<<<EOD
${msg_header}
${tag_set}
${msg_footer}
EOD;

            // 設定：返信先トゥートID
            $id_toot_replyto = $id_toot_current ?: $id_toot_original;

            // 設定：本稼働の場合は 'unlisted' -> 'public' に変更
            $visibility = (is_env_dev()) ? 'unlisted' : 'public';

            // 設定：トゥートに必要なパラメーターを設定
            $params = [
                'spoiler_text'   => $spoiler_text . $msg_additional_fraction,
                'status'         => $msg,
                'domain'         => $domain,
                'access_token'   => $access_token,
                'in_reply_to_id' => $id_toot_replyto,
                'visibility'     => $visibility,
            ];

            // 実行：とりまとめトゥート（子トゥート）の実行
            $result_toot = post_toot($params);

            // 取得：トゥート済みのトゥートID
            if (TOOT_SUCCESS == $result_toot) {
                $result_toot_value = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY);
                if (isset($result_toot_value['id'])) {
                    $id_toot_current = $result_toot_value['id'];
                } else {
                    echo_on_debug($result_toot, '子トゥートに失敗しました（$result_toot の内容）');
                }
            } else {
                echo_on_debug($params, '子トゥートに失敗しました（$params の内容）');
            }
        }
    } // EOF 宣言前の「取りまとめ」トゥート

    // -----------------------------------------------------------------
    //  宣言前トゥート
    // -----------------------------------------------------------------

    // 初期化：トゥート済みタグをリセット
    $list_tags_tooted = array();

    // 現在の日時を設定
    $date_today  = date('Y/m/d');
    $date_hour24 = date('H');
    $date_hour12 = (integer) date('h');
    // 時報用アイコン定義
    $icon_hour = strtr($date_hour12, [
        12 => '🕛', 11 => '🕚', 10 => '🕙', 9 => '🕘',
        8  => '🕗',  7 => '🕖',  6 => '🕕', 5 => '🕔',
        4  => '🕓',  3 => '🕒',  2 => '🕑', 1 => '🕐',
        0  => '🕛',
    ]);

    //
    // CW文（$spoiler_text）
    // -----------------------------------------------------------------
    $spoiler_text =<<<EOD
${icon_hour} ${date_hour24} 時をお知らせします :qiitan: (${date_today})
EOD;

    // DEV環境の場合の付加メッセージ
    $msg_additional = (is_env_dev()) ? "[#Qithub:DEV]\n" : '';

    //
    // 本文（宣言トゥートのメッセージ内容）
    // -----------------------------------------------------------------
    $msg =<<<EOD
➖➖➖➖➖➖➖➖➖➖
:qiita: 新着記事の紹介を開始します
➖➖➖➖➖➖➖➖➖➖

Qiitadon ユーザーの新規 Qiita 記事を、このトゥートに「未収載」で返信していくよ。

残りの記事は「非公開」で返信しているので興味がある人はフォローしてね。

${msg_additional}
EOD;

    // 本稼働の場合は 'unlisted' -> 'public' に変更
    $visibility = (is_env_dev()) ? 'unlisted' : 'public';

    // トゥートのパラメータ設定（新規投稿）
    $params = [
        'spoiler_text' => $spoiler_text,
        'status'       => $msg,
        'domain'       => $domain,
        'access_token' => $access_token,
        'visibility'   => $visibility,
    ];

    // 実行：宣言トゥートをポスト
    $result_toot = post_toot($params);

    // 取得：無事トゥート済みのトゥートID（親トゥート:original, 直近トゥート:current）
    //       トゥートに失敗した場合はスレッドは継続
    if (TOOT_SUCCESS == $result_toot) {
        $result_toot_value = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY);
        if (isset($result_toot_value['id'])) {
            $id_toot_original = $result_toot_value['id'];
            $id_toot_current  = $id_toot_original;
            $is_new_toot = false;
        } else {
            $debug_title = debug_msg('宣言（親トゥート）のトゥートID取得に失敗しました', true);
            echo_on_debug($result_toot, $debug_title);
            $is_new_toot = true; //スレッドを開始しない
            $id_thread   = $id_thread_last; //スレッドIDのロールバック
        }
    } else {
        $debug_title = debug_msg('宣言（親トゥート）のトゥートに失敗しました', true);
        echo_on_debug($result_toot, $debug_title);
        $is_new_toot = true; //スレッドを開始しない
        $id_thread   = $id_thread_last; //スレッドIDのロールバック
    }
}

// ---------------------------------------------------------------------
//  新着Qiita記事トゥート（スレッドの子トゥート）
// ---------------------------------------------------------------------
// 親トゥートを元に返信で新着をトゥートしてスレッドにしていく。返信は１つ
// 前のトゥートに対して行う。
// Qiita記事のタグに「テスト」や「test」が含まれる場合はスキップ。
//
// メモ：本稼働の場合、'visibility' は以下の挙動になる予定
//          初トゥート：public
//
if (! $is_new_toot && ! empty($id_toot_current) && ! empty($qiita_items_diff)) {
    // ループ：新着の差分をループしてトゥート
    foreach ($qiita_items_diff as $item) {
        // 初期化：Qiita記事の情報代入と初期化
        $item_title   = $item['title'];
        $item_url     = $item['url'];
        $item_tags    = '';
        $is_item_test = false;

        // スキップ：タイトルがテスト記事ならこの記事をスキップ
        if (is_title_test($item_title)) {
            continue;
        }

        // Qiita記事のタグにテストが含まれていた場合のタグの一時的な追加用
        // $list_tags_tooted は、とりまとめトゥートのタグ一覧
        $list_tags_tooted_tmp = $list_tags_tooted;

        // 抽出：Qiita記事のタグ（トゥートのハッシュタグ用）
        foreach ($item['tags'] as $tag) {
            $tag_name = (string) $tag['name'];

            // 一時追加：トゥート内に含めるタグ。カンマ付きのタグも処理。
            if (strpos($tag_name, ',')) {
                $tmp_tags = explode(',', $tag_name);
                foreach ($tmp_tags as $tmp_tag) {
                    $tmp_tag    = sanitize_tag_for_qiitadon($tmp_tag);
                    $item_tags .= "#${tmp_tag} ";
                    $list_tags_tooted_tmp[] = strtolower($tmp_tag);
                    // 確認：Qiita記事がテスト投稿（タグに「テスト」を含む）か確認
                    $is_item_test = (is_tag_test($tmp_tag) && ! $is_item_test) ?: $is_item_test;
                }
            } else {
                $tmp_tag = sanitize_tag_for_qiitadon($tag_name);
                $item_tags .= "#${tmp_tag} ";
                // 確認：Qiita記事がテスト投稿（タグに「テスト」を含む）か確認
                $is_item_test = (is_tag_test($tmp_tag) && ! $is_item_test) ?: $is_item_test;

                $list_tags_tooted_tmp[] = strtolower($tmp_tag);
            }

            // 整理：保存用のトゥート済みタグ一覧を一意（ユニーク）に
            $list_tags_tooted_tmp = array_values(array_unique($list_tags_tooted_tmp));
        }

        // Qiita記事がテスト記事でない場合は、tempタグ一覧を適用
        if (! $is_item_test) {
            $list_tags_tooted = $list_tags_tooted_tmp;
        }
        $item_tags = trim($item_tags);

        // スキップ：テスト記事ならトゥートをスキップ
        if ($is_item_test) {
            continue;
        }

        // 追加：DEV環境の場合の追加タグ
        $item_tags_additional = (is_env_dev()) ? "[#Qithub:DEV]" : '';

        //ユーザー名取得
        /** @todo BOTフォロワー、Qiitadonユーザーの判断。BOTフォロワーなら未収載 */
        $id_user_qiita = sanitize_id_user_qiita($item['user']['id']);

        //設定：DEV環境の時はアットマークを全角にしてメンションさせない
        $atmark = (is_env_dev()) ? '＠' : '@';

        // 作成：トゥート内容
        // Qiitadonユーザーでない場合はメッセージ内容を変更し、Boostoot
        // リンクを付加（issue #52）
        //Qiitadonユーザーとその他ユーザーでメッセージ内容を変える
        $url_user = "https://qiitadon.com/@${id_user_qiita}";
        if (isValid_url($url_user)) {
            $visibility = 'unlisted';

            //
            // QiitadonユーザーのQiita記事の場合のメッセージ
            // ---------------------------------------------------------
            $msg =<<<EOD
📰【お知らせ】:qiitan:

🆕 ${atmark}${id_user_qiita} さんが​#Qiita記事​を投稿しました！

:qiita:​『${item_title}』
${item_url}

[ ${item_tags} ]${item_tags_additional} #${id_user_qiita}
EOD;
        } else {
            $visibility = 'private';
            //
            // 基本メッセージ（Boostoot, 一般トゥート併用）
            // ---------------------------------------------------------
            $msg =<<<EOD
:qiita:​『${item_title}』
${item_url} by ${atmark}${id_user_qiita}

[ ${item_tags} ]${item_tags_additional} #${id_user_qiita}
EOD;

            // Boostootのリンク作成
            $msg_boostoot      = "\n\n${msg} #Boostoot of @qithub";
            $msg_link_boostoot = urlencode($msg_boostoot);

            //
            // 一般ユーザーのQiita記事の場合のメッセージ
            // ---------------------------------------------------------
            $msg =<<<EOD
🆕【新着Qiita記事】

​${msg}\nBoostoot! → https://qiitadon.com/share?text=${msg_link_boostoot}
EOD;
        }

        // トゥート：返信トゥート（子トゥート）の実行
        $params = [
            'status'         => $msg,
            'domain'         => $domain,
            'access_token'   => $access_token,
            'in_reply_to_id' => $id_toot_current,
            'visibility'     => $visibility,
        ];
        $result_toot = post_toot($params);

        // 取得：無事トゥート済みのトゥートID
        if (TOOT_SUCCESS == $result_toot) {
            $result_toot_value = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY);
            if (isset($result_toot_value['id'])) {
                $id_toot_current = $result_toot_value['id'];
            } else {
                echo_on_debug($result_toot, '子トゥートに失敗しました（$result_tootの内容）');
            }
        } else {
            echo_on_debug("子トゥートのトゥートに失敗しました。\n", 'Toot fail');
        }
    }
}

// ---------------------------------------------------------------------
//  保存：トゥート結果の表示とトゥートID＆今日の日付を保存
// ---------------------------------------------------------------------
if (TOOT_SUCCESS == $result_toot) {
    // 取得：子トゥートIDの取得
    //$id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];

    // 保存：$info_toot_to_save（時間、親トゥートID、最終トゥートID）
    $info_toot_to_save = [
        'id_thread'        => $id_thread,
        'id_toot_original' => $id_toot_original,
        'id_toot_current'  => $id_toot_current,
    ];
    $result_save = save_data($id_data_toot, $info_toot_to_save);
    if (SAVE_DATA_SUCCESS == $result_save) {
        echo 'Toot info saved.' . BR_EOL;
    }

    // 保存：トゥート済みQiita記事一覧
    $result_save = save_data($id_data_qiita, $qiita_items_new);
    if (SAVE_DATA_SUCCESS == $result_save) {
        echo 'Qiita info saved.' . BR_EOL;
    }

    // 保存：トゥート済みタグ一覧
    $result_save = save_data($id_data_tags, $list_tags_tooted);
    if (SAVE_DATA_SUCCESS == $result_save) {
        echo 'Tag list saved.' . BR_EOL;
    }
} else {
    echo 'Toot fail.' . BR_EOL;
    if (empty($qiita_items_diff)) {
        echo 'There is no diffs on tooted items. Did not toot.' . BR_EOL;
    } else {
        echo 'トゥートに失敗したトゥートがあります。' . BR_EOL;
    }

    if (is_env_dev()) {
        echo '<hr>';
        echo_on_debug($id_thread, 'ThreadID');
        echo_on_debug($info_toot, 'TOOT INFO');
        echo ($is_new_toot) ? 'IS NEW TOOT' . BR_EOL : 'IS THREAD' . BR_EOL;
        echo (empty($id_toot_current)) ? 'CURRENT TOOT ID EMPTY' . BR_EOL : "CUR ID:${id_toot_current}" . BR_EOL;
        echo (empty($qiita_items_diff)) ? 'NO DIFFs' . BR_EOL : 'THERE ARE DIFFs' . BR_EOL;
    }
}

// 確認：デバッグモード時の新着Qiita記事の差分確認用
if (is_mode_debug()) {
    echo '<hr>';
    echo_on_debug($qiita_items_new, 'New item');
    echo_on_debug($qiita_items_old, 'Old item');
    echo_on_debug($qiita_items_diff, 'Diff item');
    echo_on_debug($info_toot, 'TOOT INFO');
}
