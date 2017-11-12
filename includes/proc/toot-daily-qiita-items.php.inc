<?php
// プロセス 'toot-daily-qiita-items'
// -------------------------------------------------------------
// 日付ごとのスレッドで新着Qiita記事をトゥートする
//
// 'toot-daily' プロセスを新着Qiita記事のトゥートにカスタムした
// プロトタイプ。

// 初期化
$result_toot = TOOT_FAIL;

// トゥートに必要なAPIの取得
$keys_api     = get_api_keys('../../qithub.conf.json', 'qiitadon');
$domain       = $keys_api['domain'];
$access_token = ACCESS_TOKEN_MASTODON;

// トゥート済みのトゥートIDとトゥート日の読み込み
$id_data_toot = 'toot_id_and_date_of_daily_toot';
$info_toot    = load_data($id_data_toot);

// トゥート済みの新着Qiita記事の読み込み
$id_data_qiita   = 'qiita_id_tooted';
$qiita_items_old = load_data($id_data_qiita);
$qiita_items_old = ( $qiita_items_old == LOAD_DATA_EMPTY ) ? array() : $qiita_items_old;

// 動作確認用の要素の削除
//unset($qiita_items_old['9412acb2a7ff37ff9ad0']);

// 最新の新着Qiita記事の取得とトゥート済みの比較（差分取得）
$max_items  = 10;
$result_api = run_script(
    'system/get-qiita-new-items',
    ['max_items' => $max_items],
    false
);
$qiita_items_new  = decode_api_to_array($result_api)['value'];
$qiita_items_diff = array_diff_key($qiita_items_new, $qiita_items_old);

// デバッグモード時の新着Qiita記事の差分確認用
if (is_mode_debug()) {
    echo_on_debug($qiita_items_new, 'New item');
    echo_on_debug($qiita_items_old, 'Old item');
    echo_on_debug($qiita_items_diff, 'Diff item');
}

// 今日の日付をトゥート日のIDとして取得
$id_date = (int) date('YmdH');

// トゥートIDの初期化
$id_toot_current  = ''; // １つ前のトゥートID
$id_toot_original = ''; // 親のトゥートID

// 本日の初トゥートが発動済みかのフラグ設定
// 発動済みの場合、親トゥートと最新（１つ前の）トゥートを取得
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

// 新着Qiita記事トゥートの開始宣言実行（親トゥート）
//
// 本日初のトゥートが未発信の場合は、宣言トゥートを発信。
// 親トゥートと直近トゥートのIDを設定
// 本稼働の場合、'visibility' は以下の挙動になる
//    初トゥート：public
//
if ($is_new_toot) {
    // トゥート内容の作成
    $date_today = date('Y/m/d H時');
    // CW文
    $spoiler_text = "${date_today} の新着Qiita記事のトゥートを始めるよ！";
    // DEV環境の場合の付加メッセージ
    $msg_additional = (is_env_dev()) ? "[#Qithub:DEV]\n" : '';
    // 本文
    $msg =<<<EOL
このトゥートに「非公開」でトゥートしていくから、興味がある人はフォローしてね。

ただ、Qiitadon のユーザーの Qiita 記事だった場合は「未収載」でトゥートするよ。

もし、トゥートに 🔃（ブースト）や ⭐️（お気に入り）が一定数ついた場合は「公開」で改めて おすすめトゥート して、Qiita記事の方にも「いいね！」するから、気にいった記事のトゥートだったら 🔃 ⭐️ して知らせてね！${msg_additional}
EOL;
    // 本稼働の場合は 'unlisted' -> 'public' に変更
    $visibility = 'unlisted';

    // トゥートのパラメータ設定（新規投稿）
    $params = [
        'status'       => $msg,
        'spoiler_text' => $spoiler_text,
        'domain'       => $domain,
        'access_token' => $access_token,
        'visibility'   => $visibility,
    ];
    // トゥートの実行
    $result_toot = post_toot($params);

    if ($result_toot == TOOT_SUCCESS) {
        $is_new_toot = false;
        // 親トゥート・直近トゥートのID設定
        $id_toot_original = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
        $id_toot_current  = $id_toot_original;
    }
}

// 新着Qiita記事のトゥート開始（子トゥート）
//
// 親トゥートを元にスレッドで新着をトゥートしていく。返信は１つ前のトゥー
// ト、Qiita記事のタグに「テスト」や「test」が含まれる場合はスキップ。
//
// 本稼働の場合、'visibility' は以下の挙動になる
//    初トゥート：public
if (! $is_new_toot && ! empty($id_toot_current) && ! empty($qiita_items_diff)) {
    // 新着の差分をループしてトゥート
    foreach ($qiita_items_diff as $item) {
        // Qiita記事の情報代入と初期化
        $item_title   = $item['title'];
        $item_url     = $item['url'];
        $item_tags    = '';
        $is_item_test = false;

        foreach ($item['tags'] as $tag) {
            $s = (string) $tag['name'];
            $s = str_replace('++', 'プラプラ', $s);
            $s = str_replace('#', 'Sharp', $s);
            $s = str_replace('.', '', $s);
            $s = str_replace('-', '_', $s);
            $s = trim($s);

            // タグの追加
            $item_tags .= "#${s} ";

            // Qiita記事がテスト投稿（タグに「テスト」を含む）か確認
            $is_item_test = (is_tag_test($s) && ! $is_item_test) ?: $is_item_test;
        }

        // テスト記事ならトゥートをスキップ
        if ($is_item_test) {
            continue;
        }

        $item_tags = trim($item_tags);

        // DEV環境の場合にはタグに追加
        $item_tags_additional = (is_env_dev()) ? "[#Qithub:DEV]" : '';

        // トゥート内容の作成
        // Qiitadonユーザーの場合はメッセージ内容を変更
        //
        /** @todo BOTフォロワー、Qiitadonユーザーの判断
                  BOTフォロワーなら未収載 */
        $id_user_qiita = sanitize_id_user_qiita($item['user']['id']);
        $url_user = "https://qiitadon.com/@${id_user_qiita}";
        if (isValid_url($url_user)) {
            $visibility = 'unlisted';
            $msg ="【お知らせ】\n@${id_user_qiita} さんが #Qiita記事 を投稿しました！\n\n🆕『${item_title}』\n${item_url}\n\n[${item_tags}]${item_tags_additional}";
        } else {
            $visibility = 'private';
            $msg = "『${item_title}』 @${id_user_qiita}\n${item_url}\n\n[${item_tags}]${item_tags_additional}";
            $msg_link_boostoot = urlencode("🆕 #Qiita記事\n\n${msg} #Boostoot of @Qithub");
            $msg = "🆕【新着Qiita記事】\n\n${msg}\nBoostoot! → https://qiitadon.com/share?text=${msg_link_boostoot}";
        }
        
        // 返信トゥートの実行
        $params = [
            'status'         => $msg,
            'domain'         => $domain,
            'access_token'   => $access_token,
            'in_reply_to_id' => $id_toot_current,
            'visibility'     => $visibility,
        ];
        $result_toot = post_toot($params);

        if ($result_toot == TOOT_SUCCESS) {
            // トゥートIDの取得
            $id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
        }
    }
}

// トゥート結果の表示とトゥートID＆今日の日付を保存
if ($result_toot == TOOT_SUCCESS) {
    // トゥートIDの取得
    //$id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
    // 保存するデータ
    $info_toot_to_save = [
        'id_toot_current'  => $id_toot_current,
        'id_toot_original' => $id_toot_original,
        'id_date'          => $id_date,
    ];
    // 今回のトゥートIDの保存（返信の場合はデイジーチェーン）
    $result_save = save_data($id_data_toot, $info_toot_to_save);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo 'Toot info saved.' . BR_EOL;
    }
    // 今回のトゥートIDの保存（返信の場合はデイジーチェーン）
    $result_save = save_data($id_data_qiita, $qiita_items_new);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo 'Qiita info saved.' . BR_EOL;
    }
} else {
    echo 'Toot fail.' . BR_EOL;
    if (empty($qiita_items_diff)) {
        echo 'There is no diffs on tooted items. Did not toot.' . BR_EOL;
    }
}
