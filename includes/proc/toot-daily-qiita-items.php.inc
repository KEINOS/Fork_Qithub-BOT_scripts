<?php
// =====================================================================
// プロセス 'toot-daily-qiita-items'
// =====================================================================
// 定時ごとのスレッドで新着Qiita記事をトゥートする。
// 定時内の初回トゥートの場合は「宣言トゥート」を発動してから、そのトゥー
// トに返信する形でスレッドにしていく。
//

// 初期化
$result_toot      = TOOT_FAIL;
$is_new_toot      = false;
$id_toot_current  = ''; // １つ前のトゥートID
$id_toot_original = ''; // 親のトゥートID
$max_items        = 10;  // 取得するQiita記事の数

// 取得：トゥートに必要なAPI
$keys_api     = get_api_keys('../../qithub.conf.json', 'qiitadon');
$domain       = $keys_api['domain'];
$access_token = ACCESS_TOKEN_MASTODON;

// 読み込み：トゥート済みのトゥートIDとスレッドID
$id_data_toot = 'toot_id_and_date_of_daily_toot';
$info_toot    = load_data($id_data_toot);

// 読み込み：トゥート済みの新着Qiita記事ID
$id_data_qiita   = 'qiita_id_tooted';
$qiita_items_old = load_data($id_data_qiita);
$qiita_items_old = ( $qiita_items_old == LOAD_DATA_EMPTY ) ? array() : $qiita_items_old;

// 読み込み：トゥート済みタグ一覧
$id_data_tags     = 'tag_list_of_daily_toot';
$list_tags_tooted = load_data($id_data_tags);
$list_tags_tooted = ( $list_tags_tooted == LOAD_DATA_EMPTY ) ? array() : $list_tags_tooted;

// 削除：動作確認用の要素の削除
//unset($qiita_items_old['9412acb2a7ff37ff9ad0']);

// 比較・差分取得：最新の新着Qiita記事の取得とトゥート済み
$result_api = run_script(
    'system/get-qiita-new-items',
    ['max_items' => $max_items],
    false
);
$qiita_items_new  = decode_api_to_array($result_api)['value'];
$qiita_items_diff = array_diff_key($qiita_items_new, $qiita_items_old);

// 取得：現在の時間（YmdH）をスレッドのIDとして取得（１時間毎のスレッド）
$id_thread = date('Y/m/d H:00');
echo_on_debug($id_thread, 'Current Thread ID');

// フラグ設定：宣言トゥートが発動済みか
// 発動済みの場合：親トゥートと最新（１つ前の）トゥートをセット
if ($info_toot !== LOAD_DATA_EMPTY) {
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
//  宣言トゥート（スレッドの親トゥート）
// ---------------------------------------------------------------------
// とりまとめ（前回スレッド内のハッシュタグ一覧）をトゥートしたのち、次
// の新着Qiita記事トゥートのスレッドを開始する宣言トゥート（＝スレッドの
// 親トゥート）を行う。
// 本稼働の場合、'visibility' は以下の挙動になる
//    初トゥート：public
//
if ($is_new_toot) {
    //
    // 宣言前の「とりまとめ」トゥート
    // -----------------------------------------------------------------
    if (! empty($list_tags_tooted) && ! empty($id_toot_original)) {
        // トゥート済みタグ一覧
        $msg_tags = '';
        foreach ($list_tags_tooted as $tag) {
            $msg_tags .= "#${tag}\n";
        }
        // CW文
        $id_thread_last = $info_toot['id_thread'];
        $thread_date  = date("Y/m/d", strtotime($id_thread_last));
        $thread_hour  = date("H 時", strtotime($id_thread_last));
        $spoiler_text = "📋【 ${thread_hour}の :qiita: のタグ一覧】(${thread_date})";
        //
        // とりまとめトゥートのメッセージ
        //--------------------------------------------------------------
        $msg =<<<EOD
以下のタグが ${thread_date} ${thread_hour}の間に :qiita: に投稿されました。

${msg_tags}
EOD;
        // 返信先トゥートID
        $id_toot_replyto = $id_toot_current ?: $id_toot_original;
        // 本稼働の場合は 'unlisted' -> 'public' に変更
        $visibility = 'unlisted';
        // トゥート：とりまとめトゥート（子トゥート）の実行
        $params = [
            'spoiler_text'   => $spoiler_text,
            'status'         => $msg,
            'domain'         => $domain,
            'access_token'   => $access_token,
            'in_reply_to_id' => $id_toot_replyto,
            'visibility'     => $visibility,
        ];
        $result_toot = post_toot($params);

        // 取得：トゥート済みのトゥートID
        if ($result_toot == TOOT_SUCCESS) {
            $result_toot_value = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY);
            if (isset($result_toot_value['id'])) {
                $id_toot_current   = $result_toot_value['id'];
            } else {
                echo_on_debug($result_toot, '子トゥートに失敗しました（$result_tootの内容）');
            }
        } else {
            echo_on_debug("子トゥートのトゥートに失敗しました。\n", 'Toot fail');
        }
    }

    // 初期化：トゥート済みタグをリセット
    $list_tags_tooted = array();

    // トゥート時間
    $date_today  = date('Y/m/d');
    $date_hour24 = date('H');
    $date_hour12 = (integer) date('h');


    // CW文
    $icon_hour = strtr($date_hour12, [
        12 => '🕛',
        11 => '🕚',
        10 => '🕙',
        9  => '🕘',
        8  => '🕗',
        7  => '🕖',
        6  => '🕕',
        0  => '🕛',
        5  => '🕔',
        4  => '🕓',
        3  => '🕒',
        2  => '🕑',
        1  => '🕐',
        0  => '🕛',
    ]);
    $spoiler_text = "${icon_hour} ${date_hour24} 時をお知らせします:qiitan: (${date_today})";

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
    $visibility = 'unlisted';

    // トゥートのパラメータ設定（新規投稿）
    $params = [
        'spoiler_text' => $spoiler_text,
        'status'       => $msg,
        'domain'       => $domain,
        'access_token' => $access_token,
        'visibility'   => $visibility,
    ];

    // 宣言トゥートの実行
    $result_toot = post_toot($params);

    // 取得：無事トゥート済みのトゥートID（親トゥート:original, 直近トゥート:current）
    if ($result_toot == TOOT_SUCCESS) {
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
        $list_tags_tooted_tmp = $list_tags_tooted;

        // 抽出：Qiita記事のタグ（トゥートのハッシュタグ用）
        foreach ($item['tags'] as $tag) {
            $tag_name = (string) $tag['name'];

            // 一時追加：トゥート内に含めるタグ。カンマ付きのタグも処理。
            if (strpos($tag_name, ',')) {
                $tmp_tags = explode(',', $tag_name);
                foreach ($tmp_tags as $tmp_tag) {
                    $tmp_tag = sanitize_tag_for_qiitadon($tmp_tag);
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
        if (! $is_item_test) {
            $list_tags_tooted = $list_tags_tooted_tmp;
        }

        // スキップ：テスト記事ならトゥートをスキップ
        if ($is_item_test) {
            continue;
        }

        $item_tags = trim($item_tags);

        // 追加：DEV環境の場合の追加タグ
        $item_tags_additional = (is_env_dev()) ? "[#Qithub:DEV]" : '';

        // 作成：トゥート内容
        // Qiitadonユーザーでない場合はメッセージ内容を変更し、Boostoot
        // リンクを付加（issue #52）
        //
        /** @todo BOTフォロワー、Qiitadonユーザーの判断。BOTフォロワーなら未収載 */
        $id_user_qiita = sanitize_id_user_qiita($item['user']['id']);
        $atmark = (is_env_dev()) ? '＠' : '@'; //DEV環境の時はメンションさせない
        $url_user = "https://qiitadon.com/@${id_user_qiita}";
        if (isValid_url($url_user)) {
            $visibility = 'unlisted';

            //
            // QiitadonユーザーのQiita記事の場合のメッセージ
            // ---------------------------------------------------------
            $msg =<<<EOD
📰【お知らせ】:qiitan:

${atmark}${id_user_qiita} さんが #Qiita記事 を投稿しました！

🆕:qiita:『${item_title}』
${item_url}

[ ${item_tags} ]${item_tags_additional}
EOD;
        } else {
            $visibility = 'private';
            //
            // 基本メッセージ（Boostoot, 一般トゥート併用）
            // ---------------------------------------------------------
            $msg =<<<EOD
『${item_title}』 ${atmark}${id_user_qiita}
${item_url}

[ ${item_tags} ]${item_tags_additional}
EOD;

            // Boostootのリンク作成
            $msg_boostoot      = "この #Qiita記事 :qiita: おすすめです！ :thumbsup: \n\n${msg} #Boostoot of @Qithub";
            $msg_link_boostoot = urlencode($msg_boostoot);

            //
            // 一般ユーザーのQiita記事の場合のメッセージ
            // ---------------------------------------------------------
            $msg =<<<EOD
🆕【新着Qiita記事】:qiita:

${msg}\nBoostoot! → https://qiitadon.com/share?text=${msg_link_boostoot}
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
        if ($result_toot == TOOT_SUCCESS) {
            $result_toot_value = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY);
            if (isset($result_toot_value['id'])) {
                $id_toot_current   = $result_toot_value['id'];
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
if ($result_toot == TOOT_SUCCESS) {
    // 取得：子トゥートIDの取得
    //$id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];

    // 保存：$info_toot_to_save（時間、親トゥートID、最終トゥートID）
    $info_toot_to_save = [
        'id_thread'        => $id_thread,
        'id_toot_original' => $id_toot_original,
        'id_toot_current'  => $id_toot_current,
    ];
    $result_save = save_data($id_data_toot, $info_toot_to_save);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo 'Toot info saved.' . BR_EOL;
    }

    // 保存：トゥート済みQiita記事一覧
    $result_save = save_data($id_data_qiita, $qiita_items_new);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo 'Qiita info saved.' . BR_EOL;
    }

    // 保存：トゥート済みタグ一覧
    $result_save = save_data($id_data_tags, $list_tags_tooted);
    if ($result_save == SAVE_DATA_SUCCESS) {
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
