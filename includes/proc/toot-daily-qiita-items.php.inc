<?php
// プロセス 'toot-daily-qiita-items'
// ---------------------------------------------------------------------
// 定時ごとのスレッドで新着Qiita記事をトゥートする。
// 定時内の初回トゥートの場合は「宣言トゥート」を発動してから、そのトゥー
// トに返信する形でスレッドにしていく。
//

// 初期化
$result_toot      = TOOT_FAIL;
$id_toot_current  = ''; // １つ前のトゥートID
$id_toot_original = ''; // 親のトゥートID

// 取得：トゥートに必要なAPI
$keys_api     = get_api_keys('../../qithub.conf.json', 'qiitadon');
$domain       = $keys_api['domain'];
$access_token = ACCESS_TOKEN_MASTODON;

// 読み込み：トゥートIDとトゥート日（トゥート済み）
$id_data_toot = 'toot_id_and_date_of_daily_toot';
$info_toot    = load_data($id_data_toot);

// 読み込み：新着Qiita記事ID（トゥート済み）
$id_data_qiita   = 'qiita_id_tooted';
$qiita_items_old = load_data($id_data_qiita);
$qiita_items_old = ( $qiita_items_old == LOAD_DATA_EMPTY ) ? array() : $qiita_items_old;

// 削除：動作確認用の要素の削除
// 　　　アンコメントすると指定したトゥートIDがトゥート済みから削除され、
// 　　　新着一覧にまだ存在する場合は、再トゥートされる。
//unset($qiita_items_old['9412acb2a7ff37ff9ad0']);

// 比較・差分取得：最新の新着Qiita記事の取得とトゥート済み
$max_items  = 10;  //ここはハードコードでいいのか？
$result_api = run_script(
    'system/get-qiita-new-items',
    ['max_items' => $max_items],
    false
);
$qiita_items_new  = decode_api_to_array($result_api)['value'];
$qiita_items_diff = array_diff_key($qiita_items_new, $qiita_items_old);

// 確認：デバッグモード時の新着Qiita記事の差分確認用
if (is_mode_debug()) {
    echo_on_debug($qiita_items_new, 'New item');
    echo_on_debug($qiita_items_old, 'Old item');
    echo_on_debug($qiita_items_diff, 'Diff item');
}

// 取得：今日の日付をトゥート日のIDとして取得
$id_date = (int) date('YmdH');

// フラグ設定：宣言トゥートが発動済みか
// 発動済みの場合：親トゥートと最新（１つ前の）トゥートをセット
if ($info_toot !== LOAD_DATA_EMPTY) {
    // 宣言トゥート済みフラグを立てる（保存日の比較）
    $is_new_toot = ($info_toot['id_date'] !== $id_date);
    // トゥートIDの取得
    $id_toot_current  = $info_toot['id_toot_current'];
    $id_toot_original = $info_toot['id_toot_original'];
} else {
    // 宣言トゥートフラグ
    $is_new_toot = true;
}

// 宣言トゥート（親トゥート）
//
// 新着Qiita記事トゥートの開始の宣言トゥート（＝スレッドの親トゥート）
// 宣言トゥートが未発信の場合は、宣言トゥートを発信し、親トゥートと直近
// トゥートのIDを設定
// 本稼働の場合、'visibility' は以下の挙動になる
//    初トゥート：public
//
if ($is_new_toot) {
    // トゥート時間
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
    // 宣言トゥートの実行
    $result_toot = post_toot($params);
    // 取得：無事トゥート済みのトゥートID（親トゥート:original, 直近トゥート:current）
    if ($result_toot == TOOT_SUCCESS) {
        $is_new_toot = false;
        $id_toot_original = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
        $id_toot_current  = $id_toot_original;
    }
    /** @todo トゥート失敗時の処理 issue #46 おそらくpost_tootの戻り値がおかしい */
}

// 新着Qiita記事トゥート（子トゥート）
//
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

        foreach ($item['tags'] as $tag) {
            $s = (string) $tag['name'];
            $s = str_replace('++', 'プラプラ', $s);
            $s = str_replace('#', 'Sharp', $s);
            $s = str_replace('.', '', $s);
            $s = str_replace('-', '_', $s);
            $s = trim($s);

            // 追加：トゥート内に含めるタグ一覧
            $item_tags .= "#${s} ";

            // 確認：Qiita記事がテスト投稿（タグに「テスト」を含む）か確認
            $is_item_test = (is_tag_test($s) && ! $is_item_test) ?: $is_item_test;
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
        $url_user = "https://qiitadon.com/@${id_user_qiita}";
        if (isValid_url($url_user)) {
            $visibility = 'unlisted';
            $msg ="【お知らせ】\n@${id_user_qiita} さんが #Qiita記事 を投稿しました！\n\n🆕『${item_title}』\n${item_url}\n\n[${item_tags}]${item_tags_additional}";
        } else {
            $visibility = 'private';
            $msg = "『${item_title}』 @${id_user_qiita}\n${item_url}\n\n[${item_tags}]${item_tags_additional}";
            // Boostoot
            $msg = "🆕【新着Qiita記事】\n\n${msg}\nBoostoot! → https://qiitadon.com/share?text=${msg_link_boostoot}";
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
            $id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
        }
    }
}

// トゥート結果の表示とトゥートID＆今日の日付を保存
if ($result_toot == TOOT_SUCCESS) {
    // 取得：子トゥートIDの取得
    //$id_toot_current = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];

    // 設定：保存するデータ（時間、親トゥートID、最終トゥートID）
    $info_toot_to_save = [
        'id_date'          => $id_date,
        'id_toot_original' => $id_toot_original,
        'id_toot_current'  => $id_toot_current,
    ];
    // 保存：$info_toot_to_save
    $result_save = save_data($id_data_toot, $info_toot_to_save);
    if ($result_save == SAVE_DATA_SUCCESS) {
        echo 'Toot info saved.' . BR_EOL;
    }
    // 保存：トゥート済みQiita記事一覧
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
