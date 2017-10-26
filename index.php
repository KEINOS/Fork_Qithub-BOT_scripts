<?php
error_reporting(E_ALL);
/**
 * Front to the 「Qithub」 application.
 *
 * This application is a BOT that operate of, by and for the Qiita users.
 * See the link below for details.
 *
 * @copyright Qithub Organization All Rights Reserved
 * @license 2017/10/06 under discussion
 * @author @KEINOS@Qiitadon.com
 * @link https://github.com/Qithub-BOT/
 * @php version >=5.4.45
 */


/* =====================================================================
    初期設定
   ===================================================================== */

// 言語設定->日本語 地域->東京 にセット
set_utf8_ja('Asia/Tokyo');

// 定数設定エリア
define('IS_MODE_DEBUG', isset($_GET['mode']) and ($_GET['mode'] == 'debug'));
define('IS_PROC_REGULAR', ! isset($_GET['process'])); // 定例処理
define('IS_PROC_DEMAND', isset($_GET['process']));    // 随時処理
define('DIR_SEP', DIRECTORY_SEPARATOR);
define('BR_EOL', '<br>' . PHP_EOL);
define('LOAD_DATA_EMPTY', false);
define('SAVE_DATA_SUCCESS', true);
define('SAVE_DATA_FAIL', false);
define('TOOT_SUCCESS', true);
define('TOOT_FAIL', false);

// 'system' および 'plugin' が使えるプログラム言語
$extension_types = [
        'php'=>'.php',
        'python' =>'.py',
    ];

/* =====================================================================
    メイン
   ===================================================================== */

/* ---------------------------------
    定例処理
   --------------------------------- */
if (IS_PROC_REGULAR) {
    echo "定例処理を行う予定（in progress）";
    //TODO 最新Qiita記事の取得
} else {
/* ---------------------------------
    随時処理
    クエリの'process'値で分岐処理
   --------------------------------- */
    switch (strtolower($_GET['process'])) {

        // 'github'
        // -------------------------------------------------------------
        // GitHub からの WebHook 処理
        // クエリの'method'オプションが指定されていない場合は受け取った
        // データを保存。'method'オプションによって保存データの閲覧・削
        // 除を行う
        case 'github':
            // データを保存するキー（データID）
            $id_data = 'WebHook-GitHub';

            // 保存済みデータの読み込み
            $log_data = load_data($id_data);
            if ($log_data === LOAD_DATA_EMPTY) {
                $log_data = array();
            }

            // ログの操作（view, delete）
            // 'method'オプションによる保存データの扱い
            if (isset($_GET['method'])) {
                switch ($_GET['method']) {
                    // ログの一部削除
                    // ログのキー（=タイムスタンプ）を指定して削除
                    case 'delete':
                        $key_to_delete = $_GET['key'];
                        // 削除の実行（データのアップデート）
                        if (isset($log_data[$key_to_delete])) {
                            unset($log_data[$key_to_delete]);
                            if (save_data($id_data, $log_data)) {
                                $log_data = load_data($id_data);
                            } else {
                                $log_data = 'Error on updating log';
                            }

                            echo '<pre style=\'width:100%;overflow: auto;white-space: pre-wrap; word-wrap: break-word;\'>' . PHP_EOL;
                            print_r($log_data);
                            echo '</pre>' . PHP_EOL;
                        } else {
                            echo "削除するデータのキーが指定されていません。";
                        }
                        break;

                    // ログ表示（WebHook からのデータの保存内容の確認）
                    case 'view':
                        echo '<pre style=\'width:100%;overflow: auto;white-space: pre-wrap; word-wrap: break-word;\'>' . PHP_EOL;
                        print_r($log_data);
                        echo '</pre>' . PHP_EOL;
                        break;
                    default:
                        break;
                }
                die();

            // WebHook からの受け取りデータの保存
            } else {
                // データの保存キー
                $timestamp = date("Ymd-His");
                // データの作成
                $log_data[$timestamp] = [
                    'getallheaders' => getallheaders(),
                    'get'           => $_GET,
                    'post'          => $_POST,
                    'ip'            => $_SERVER["REMOTE_ADDR"],
                    'host'          => gethostbyaddr($_SERVER["REMOTE_ADDR"]),
                    'raw_post_data' => file_get_contents('php://input'),
                ];
                // 保存実行
                $result = save_data($id_data, $log_data);
                // レスポンス
                if ($result==true) {
                    echo "Data saved." . PHP_EOL;
                    echo "Data ID/key was: ${id_data}/${timestamp}" . PHP_EOL;
                }
            }

            break;

        // 'sample'
        // -------------------------------------------------------------
        // BOT のトリガーテスト（プロセス）の動作サンプル
        //
        // 基本スクリプトでデータ保存・読み込みを行う。データの保存自体
        // は'system/data-io' をラッパー関数で利用する例。
        case 'sample':
            // サンプルデータのデータID
            $id_data = 'sample';

            // サンプルデータの作成と保存
            $sample_data = [
                'time_stamp' => date("Y/m/d H:i:s"),
                'hoge'       => 'hoge',
            ];
            save_data($id_data, $sample_data);

            // サンプルデータの読み込み
            $result = load_data($id_data);
            print_r($result);
            break;

        // 'say-hello-world'
        // -------------------------------------------------------------
        // 'plugins/say-hello-world' を利用したサンプル
        //
        // 'system/data-io','system/delete-toot','system/post-toot'の
        // ラッパー関数を使って、前回トゥートしたトゥートを削除し、
        // 'say-hello-world'プラグインより取得したメッセージを新規トゥー
        // トする。
        case 'say-hello-world':
            // トゥートIDの保存キー（データID）
            $id_data = 'last-toot-id_say-hello-world';

            // 前回トゥートのIDを取得
            $id_pre_toot  = load_data($id_data);
            $has_pre_toot = ($id_pre_toot !== LOAD_DATA_EMPTY);

            // トゥートに必要なAPIの取得
            $keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');

            // 前回トゥートを削除
            $msg_toot_deleted = '';
            $is_toot_deleted  = false;
            if ($has_pre_toot) {
                $is_toot_deleted = delete_toot([
                    'domain'       => $keys_api['domain'],
                    'access_token' => $keys_api['access_token'],
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
                    'access_token' => $keys_api['access_token'],
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

            break;

        // 'get-qiita-new-items'
        // -------------------------------------------------------------
        // Qiita記事の新着N件を表示するサンプル
        //
        // クエリのパラメーター'max_items'が指定されている場合はその件数
        // ぶん、未指定の場合はデフォルトの5件が表示される。専用のラッパー
        // を作らず汎用的な呼び出し方法で 'system/get-qiita-new-items'を
        // 利用するサンプル
        case 'get-qiita-new-items':
            if (isset($_GET['max_items'])) {
                $max_items = (int) $_GET['max_items'];
            } else {
                $max_items = 5;
            }

            $params = [
                'max_items' => $max_items,
            ];

            $result_api = run_script('system/get-qiita-new-items', $params, false);
            $result     = decode_api_to_array($result_api);

            if ($result['result']) {
                echo '<pre>';
                print_r($result);
                echo '</pre>';
            }

            break;
        
        // 'toot-daily'
        // -------------------------------------------------------------
        // 日付ごとのスレッドでトゥートするサンプル
        //
        // 定例処理用のプロトタイプ。トゥートした日付の初トゥートの場合
        // は普通にトゥート（親トゥート）し、以降の同日トゥートは返信で
        // トゥート（子トゥート）します。
        // 子トゥートは親に対しての返信でなく、１つ前のトゥートに対して
        // 返信される。（トゥートクリック時にスレッド内容がわかるように）
        case 'toot-daily':
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
                    'access_token' => $keys_api['access_token'],
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
                    'access_token'   => $keys_api['access_token'],
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

            break; // EOF toot-daily

        // 'toot-daily-qiita-items'
        // -------------------------------------------------------------
        // 日付ごとのスレッドで新着Qiita記事をトゥートするサンプル
        // 'toot-daily' プロセスを新着Qiita記事のトゥートにカスタムした
        // プロトタイプ。
        case 'toot-daily-qiita-items':
            // トゥートに必要なAPIの取得
            $keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');
            $domain       = $keys_api['domain'];
            $access_token = $keys_api['access_token'];

            // トゥート済みのトゥートIDとトゥート日の読み込み
            $id_data_toot = 'toot_id_and_date_of_daily_toot';
            $info_toot = load_data($id_data_toot);

            // トゥート済みの新着Qiita記事の読み込み
            $id_data_qiita   = 'qiita_id_tooted';
            $qiita_items_old = load_data($id_data_qiita);
            $qiita_items_old = ( $qiita_items_old == LOAD_DATA_EMPTY ) ? array() : $qiita_items_old;
            
            // 動作確認用の要素の削除
            //unset($qiita_items_old['cf93b63acc8a313c88bb']);

            // 最新の新着Qiita記事の取得とトゥート済みの比較（差分取得）
            $max_items  = 10;
            $result_api = run_script(
                'system/get-qiita-new-items',
                ['max_items' => $max_items],
                false
            );
            $qiita_items_new  = decode_api_to_array($result_api)['value'];
            $qiita_items_diff = array_diff_key($qiita_items_new, $qiita_items_old);
            
            if(IS_MODE_DEBUG){
                echo '<pre>';
                print_r($qiita_items_diff);
                echo '</pre>';
                die;
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
                $date_today = date('Y/m/d H時    ');
                $msg = "${date_today} の新着Qiita記事のトゥートを始めるよ！";
                $spoiler_text =<<<EOL
このトゥートに「非公開」でトゥートしていくから、興味がある人はフォローしてね。

ただ、QiitadonのユーザーのQiita記事だった場合は「未収載」でトゥートするよ。
また、「★（お気に入り）」や「ブースト」数が一定数超えたトゥートは「公開」で改めてトゥートするね。
Qiita記事にも「いいね」するから、気にいった記事のトゥートだったら知らせてね！
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
                
                if ($result_toot == TOOT_SUCCESS){
                    $is_new_toot = false;
                    // 親トゥート・直近トゥートのID設定
                    $id_toot_original = json_decode($result_toot['value'], JSON_OBJECT_AS_ARRAY)['id'];
                    $id_toot_current  = $id_toot_original;
                }

            }

            // 新着Qiita記事のトゥート開始（子トゥート）
            //
            // 親トゥートを元にスレッドで新着をトゥートしていく。返信は
            // １つ前のトゥートにしていく。
            // 
            // 本稼働の場合、'visibility' は以下の挙動になる
            //    初トゥート：public
            if (! $is_new_toot && ! empty($id_toot_current) && ! empty($qiita_items_diff)) {

                // 新着の差分をループしてトゥート
                foreach($qiita_items_diff as $item){

                    // Qiita記事の情報取得
                    $item_title = $item['title'];
                    $item_url   = $item['url'];
                    $item_tags  = '';
                    foreach($item['tags'] as $tag){
                        $s = $tag['name'];
                        $s = str_replace('#', 'Sharp', $s);
                        $s = str_replace('.', '', $s);
                        $s = str_replace('-', '_', $s);
                        $s = trim($s);

                        $item_tags .= "#${s} ";
                    }
                    $item_tags = trim($item_tags);

                    // トゥート内容の作成
                    // Qiitadonユーザーの場合はメッセージ内容を変更
                    //
                    /** @todo BOTフォロワー、Qiitadonユーザーの判断
                              BOTフォロワーなら未収載 */
                    $id_user_qiita = sanitize_id_user_qiita($item['user']['id']);
                    $url_user = "https://qiitadon.com/@${id_user_qiita}";
                    if(isValid_url($url_user)){
                        $visibility = 'unlisted';
                        $msg ="@${id_user_qiita} さんが新規Qiita記事を投稿しました。\n\n『${item_title}』\n${item_url}\n${item_tags}";
                    } else {
                        $visibility = 'private';
                        $msg ="新しいQiita記事が投稿されました。\n\n『${item_title}』 @${id_user_qiita}\n${item_url}\n${item_tags}";
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
                    echo "Toot info saved." . BR_EOL;
                }
                // 今回のトゥートIDの保存（返信の場合はデイジーチェーン）
                $result_save = save_data($id_data_qiita, $qiita_items_diff);
                if ($result_save == SAVE_DATA_SUCCESS) {
                    echo "Qiita info saved." . BR_EOL;
                }

            } else {
                echo "Toot fail." . BR_EOL;
            }

            break; //EOF toot-daily-qiita-items

        // get-mastodon-user-info
        // -------------------------------------------------------------
        // マストドンのユーザーアカウントおよびフォロワーの情報を表示する
        //
        // 一度取得した情報はキャッシュされる。リクエスト・パラメータ
        // 'use_cash' に `false` が指定されていた場合は新規取得（更新）
        // される。
        // パラメーター 'id' にユーザーID が指定されていた場合は、そのユー
        // ザーおよびフォロワーの情報が表示される。
        // また、'id', 'use_cash', 'mode'のオプション・パラメータは本体
        // スクリプトのURLクエリと連動しており以下のように受け取ることが
        // できる。
        //
        //   BOT のアカウントを取得する例（JSON形式）
        //       /qithub/?process=get-mastodon-user-info
        //   BOT のアカウントを取得する例（デバッグモード,配列表示）
        //       /qithub/?process=get-mastodon-user-info&mode=debug
        //   指定したユーザーのアカウントを取得する例
        //       /qithub/?process=get-mastodon-user-info&id=3835
        case 'get-mastodon-user-info':
            // Mastodon API に必要なキーの取得
            $keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');

            // 基本のパラメーター設定
            $params = [
                'domain'       => $keys_api['domain'],
                'access_token' => $keys_api['access_token'],
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

            break;

        default:
            echo 'その他の処理';
    }
}

die(); // END of MAIN

/* =====================================================================
    Functions
   ===================================================================== */
/**
 * 'system' および 'plugin' のスクリプトを実行します。
 *
 * 実行に必要なパラメーターは各スクリプトのディレクトリにある README.md
 * を参照して準拠してください。
 *
 * @param  string  $dir_name        スクリプトのディレクトリ名。
 *                                  エンドポイント名。
 *                                  'system/<script name>'
 *                                  'plugin/<script name>'
 * @param  array   $params          スクリプトに渡す配列（パラメーター）
 * @param  boolean $run_background  trueの場合、実行結果を待たずにバック
 *                                  グラウンドで実行します
 * @return json or boolean          バックグラウンド実行の場合は常にtrue
 */
function run_script($dir_name, $params, $run_background = true)
{
    $lang_type = get_lang_type($dir_name);
    $command   = get_cli_command(
        $lang_type,
        $dir_name,
        $params
    );

    // Run as background script unless debug mode.
    // NOTE: Be careful that commands are NOT single quoted
    //       but grave accent aka backquote/backtick.
    if ($run_background) {
        $log    = `$command > /dev/null &`;
        $result = 'OK';
    } else {
        $result = `$command`;
    }

    return $result;
}

/**
 *  URLエンコードされたJSONを配列にデコードします
 *
 * @param  string  $json_enc APIからのエンコード済み出力結果
 * @return array             デコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function decode_api_to_array($json_enc)
{
    $json_raw  = urldecode($json_enc);
    $array     = json_decode($json_raw, JSON_OBJECT_AS_ARRAY);

    return $array;
}

/**
 *  配列をJSON & URLエンコードにエンコードします
 *
 *  'system'および'plugins'は、このデータ形式で入力を受け付けます。
 *
 * @param  array  $array_value スクリプトの実行言語（'php','python'）
 * @return string              エンコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function encode_array_to_api($array_value)
{
    $array['is_mode_debug'] = IS_MODE_DEBUG;
    $array['values']        = $array_value;

    $json_raw = json_encode($array_value);
    $json_enc = urlencode($json_raw);

    return $json_enc;
}

/* ---------------------------------
    ゲッター系 Functions
   --------------------------------- */
/**
 *  指定された 'system' および 'pugins' の スクリプトの実行言語を取得し
 *  ます。
 *
 *  第１引数に渡されたディレクトリ名より "main.xxx" の拡張子を調べ、該当
 *  する言語を返します。
 *
 * @param  string $dir_name  スクリプトのディレクトリ名
 * @return string            プログラム言語名。false の場合は引数に問題
 *                           があるか定義されていない拡張子が"main.xxxx"
 *                           に使われています。
 */
function get_lang_type($dir_name)
{
    global $extension_types;

    $path_basic = "./${dir_name}/main";

    foreach ($extension_types as $lang => $ext) {
        if (file_exists($path_basic . $ext)) {
            $result_lang = $lang;
            break;
        } else {
            debug_msg("Can't determine extension type. File not exist at '${path_basic}'.");
            $result_lang = false;
        }
    }

    return $result_lang;
}

/**
 * 各種API用に必要なキーの取得をします
 *
 * 第１引数に渡されたパスにあるJSON形式のCONFファイルの中から第２引数の
 * キーを持つ配列の値を返します。
 *
 * @param  string $path_file_conf
 * @param  string $name_conf
 * @return array
 */
function get_api_keys($path_file_conf, $name_conf)
{
    $conf_json  = file_get_contents(trim($path_file_conf));
    $conf_array = json_decode($conf_json, JSON_OBJECT_AS_ARRAY);
    $keys_api   = $conf_array[$name_conf];

    return $keys_api;
}

/**
 * 'system'および'plugins' を CLI (Command Line Interface)で実行するため
 * の Qithub 準拠のコマンドを生成します
 *
 * @param  string $lang_type   スクリプトの実行言語（'php','python'）
 * @param  string $name_script CLIで実行するスクリプトのパス
 * @param  string $array_value CLIでスクリプトに渡す引数の配列データ
 * @return string              CLI用のコマンド
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function get_cli_command($lang_type, $dir_name, $array_value)
{
    $path_cli    = get_path_exe($lang_type);
    $path_script = get_path_script($dir_name, $lang_type);
    $argument    = encode_array_to_api($array_value);
    $command     = "${path_cli} ${path_script} ${argument}";

    return  $command;
}


/**
 * 'system' および 'plugins' のスクリプトの絶対パスを返します
 *
 * @param  string $dir_name    スクリプトのディレクトリ
 *                             'system/<script name>'
 *                             'plugin/<script name>'
 * @param  string $lang_type   スクリプトの実行言語（'php','python',etc）
 * @return string              スクリプトの絶対パス
 */
function get_path_script($dir_name, $lang_type)
{
    global $extension_types;

    $ext = $extension_types[$lang_type];

    $path_dir_scripts = '.' . DIR_SEP .$dir_name . DIR_SEP;
    $path_file_script = $path_dir_scripts . 'main' . $ext;

    if (! file_exists($path_file_script)) {
        throw new Exception("不正なファイルパスの指定 ${path_file_script}");
        $path_file_script = "./unknown_path";
    } else {
        $path_file_script = realpath($path_file_script);
    }

    return $path_file_script;
}

/**
 *  スクリプトを CLI で実行する際に必要なプログラム言語のパスを取得する
 *
 * @param  string $lang_type スクリプトの実行言語（'php','python',etc）
 * @return string            スクリプト言語のパス
 */
function get_path_exe($lang_type)
{
    $lang_type = strtolower($lang_type);
    switch ($lang_type) {
        case 'php':
            $path_cli = '/usr/bin/php'; // PHP7 .0.22 cli
            break;
        default:
            throw new Exception("不明なプログラム言語の指定 ${lang_type}");
            $path_cli = null;
            break;
    }

    return $path_cli;
}
/* ---------------------------------
    Plugin Functions
    'plugins/xxxxxx'のQithub API ラッパー
   --------------------------------- */
function delete_toot($params)
{
    $result_api = run_script('system/delete-toot', $params, false);
    $result     = decode_api_to_array($result_api);

    return  ( $result['result'] == 'OK' );
}

function post_toot($params)
{
    $result_api = run_script('system/post-toot', $params, false);
    $result     = decode_api_to_array($result_api);

    return  ( $result['result'] == 'OK' ) ? $result : false;
}


/* ---------------------------------
    DATA I/O Functions
    'system/data-io'のQithub API ラッパー
   --------------------------------- */
/**
 *  データの読み込みをします
 *
 * @param  string  $id_data 保存したデータのキー
 * @return mixed            保存したデータ
 */
function load_data($id_data)
{
    $params   = [
        'command' => 'load',
        'id'      => $id_data,
    ];
    $result_api = run_script('system/data-io', $params, false);
    $result     = decode_api_to_array($result_api);
    if ($result['result'] == 'OK') {
        return $result['value'];
    } else {
        $msg_error =<<<EOL
【読み込みエラー】
Data ID：'${id_data}' でのデータ読み込み時にエラーが発生しました。<br>
【エラー内容】${result['value']}
EOL;
        debug_msg($msg_error);
        return LOAD_DATA_EMPTY;
    }
}

/**
 *  データの書き込みをします
 *
 * @param  string  $id_data 保存したデータのキー
 * @param  mixed   $data    保存したいデータ
 * @return boolean          データの保存成功は true、失敗は false
 */
function save_data($id_data, $data)
{
    $params = [
        'command' => 'save',
        'id'      => $id_data,
        'value'   => $data,
    ];
    $result_api = run_script('system/data-io', $params, false);
    $result     = decode_api_to_array($result_api);

    return ($result['result'] == 'OK') ? SAVE_DATA_SUCCESS : SAVE_DATA_FAIL;
}

/**
 *  データの削除をします
 *
 * @param  string  $id_data 保存したデータのキー
 * @return boolean          削除されれば true
 */
function delete_data($id_data)
{
    $params   = [
        'command' => 'delete',
        'id'      => $id_data,
    ];
    $result_api = run_script('system/data-io', $params, false);
    $result     = decode_api_to_array($result_api);
    if ($result['result'] == 'OK') {
        return $result['value'];
    } else {
        throw new Exception("不正なデータIDです：\n ID：${id_data}\n 内容：${result['value']}");
        return false;
    }
}


/* ---------------------------------
    バリデーション Functions
   --------------------------------- */

/**
 * isValid_url function.
 *
 * 有効なURLかを返す。主にQiitadonユーザーが存在するかの確認に利用。
 * 
 * @access public
 * @param mixed $sUrl
 * @return void
 */
function isValid_url($sUrl)
{
    if (( $sResponce = @file_get_contents($sUrl) ) == false) {
        // HTTPステータスコードを取得する
        list($version,$status_code,$msg) = explode(' ', $http_response_header[0], 3);
        // ステータスコードごとの処理
        switch ($status_code) {
            case 404:
                //echo "404: Not found.";
                return false;
                break;
            default:
                //return "ERROR ${status_code}";
                return false;
                break;
        }
    } else {
        //正常にfile_get_contentsで取得できた場合の処理
        //echo "OK: {$sResponce}";
        return true;
    }
}

/**
 * sanitize_id_user_qiita function.
 *
 * ユーザーのQiitaIDがQiitadonIDと同じであるとは限らないため、Qiitadonの
 * 禁則文字の置換処理を行う。
 * @access public
 * @param mixed $s
 * @return void
 * @todo QiitaIDとQiitadonIDが違う人がいるので置き換えが必要。
 * @note BOTのフォロワー情報からQiitaIDは取得できる 
 */
function sanitize_id_user_qiita($s){
    $s = trim($s);
    
    $table = [
        'sample-user' => 'sample_user',
    ];
    $search  = array_keys($table);
    $replace = array_values($table);

    /* Common replacement */
    $s = str_replace($search, $replace, $s);
    $s = str_replace('@github', '', $s);
    $s = str_replace('@', 'at', $s);
    $s = str_replace('.', '_', $s);
    $s = str_replace('.', '_', $s);
    $s = str_replace('-', '_', $s);
    
    return $s;
};


/* ---------------------------------
    環境／デバッグ用／その他 Functions
   --------------------------------- */


/**
 * with function.
 *
 * 定数の文字列内展開用関数
 *
 * @access public
 * @param mixed $v
 * @return mixed
 */
function with($v)
{
    return $v;
}
$with = "with";

/**
 * debug_msg function.
 *
 * デバッグモード（IS_MODE_DEBUG = true）時のみ、try, catchできるエラー
 * （E_USER_WARNING）出力関数。エラー発生元の取得も行う。
 *
 * @access public
 * @param mixed $str
 * @return void
 */
function debug_msg($str)
{
    $with = "with";
    if (IS_MODE_DEBUG) {
        $line = debug_backtrace()[1]['line'];
        //$line = print_r(debug_backtrace(),true);
        trigger_error(
            "${str}【呼び出し元】 ${line}行{$with(BR_EOL)}",
            E_USER_WARNING
        );
    }
}

/**
 * echo_on_debug function.
 *
 * デバッグモード（IS_MODE_DEBUG = true）時のみ echo/print_r を返す関数。
 * 主にテスト動作で引数をチェックするなどに利用する。
 *
 * @access public
 * @param  mixed  $expression
 * @return void
 * @SuppressWarnings(PHPMD)
 */
function echo_on_debug($expression)
{
    if (IS_MODE_DEBUG) {
        if (is_string($expression)) {
            echo "<pre>${expression}</pre>" . PHP_EOL;
        } else {
            echo '<pre>';
            print_r($expression);
            echo "</pre>" . PHP_EOL;
        }
    }
}

/**
 *  実行環境の言語を日本語に設定しタイムゾーンを設定する
 *
 * @param  string  $timezone デフォルトは東京
 * @return
 * @SuppressWarnings(PHPMD)
 */
function set_utf8_ja($timezone = 'Asia/Tokyo')
{
    if (! function_exists('mb_language')) {
        die('This application requires mb_language.');
    }

    date_default_timezone_set($timezone);
    setlocale(LC_ALL, 'ja_JP');
    mb_language('ja');
    mb_internal_encoding('UTF-8');
    mb_http_output('UTF-8');
    ob_start("mb_output_handler");
}
