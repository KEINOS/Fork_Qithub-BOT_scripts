<?php error_reporting(E_ALL);

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

// 定数読み込み設定
include_once('./includes/constants.php.inc');

// 関数読み込み設定
include_once('./includes/functions.php.inc');

/* =====================================================================
    初期設定
   ===================================================================== */

// 言語設定->日本語 地域->東京 にセット
set_utf8_ja('Asia/Tokyo');

// 'system' および 'plugin' が使えるプログラム言語
$extension_types = [
        'php'=>'.php',
        'python' =>'.py',
    ];

/* =====================================================================
    メイン
   ===================================================================== */

   /* ------------------------------------------------------------------
       定例処理

       cron により5分おきに実行される内容です。
   --------------------------------------------------------------------- */
if (IS_PROC_REGULAR) {
    echo "定例処理を行う予定（in progress）";
    /** TODO 最新Qiita記事の取得 */
} else {
   /* ------------------------------------------------------------------
       随時処理 / 臨時処理

       WebHook や テスト用にダイレクトに実行される内容です。
       各処理は、リクエストされたクエリの'process'値で分岐処理されます。
   --------------------------------------------------------------------- */
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
            include_once('./includes/toot-daily-qiita-items.php.inc');
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

        // 'dice-roll'
        // -------------------------------------------------------------
        // サイコロを振った結果と合計を表示するプラグイン（ PR #38）の
        // 動作テスト。
        //
        // クエリの引数オプション
        //   '&times='     ：振る回数
        //   '&max='       ：サイコロの最大出目
        //   '&dicecode='  ：ダイスコード
        //   '&mode=debug' ：デバッグモード（詳細表示）
        case 'dice-roll':
            // Number of fling（サイコロを振る回数）
            $times = '1';
            if (isset($_GET['times']) && is_numeric($_GET['times']) && ! empty($_GET['times'])) {
                $times = (integer) $_GET['times'];
            }
            // Number of side of a dice（サイコロの最大出目）
            $max_side = '6';
            if (isset($_GET['max']) && is_numeric($_GET['max']) && ! empty($_GET['max'])) {
                $max_side = intval($_GET['max']);
            }
            // Set 'dicecode'
            $dicecode = isset($_GET['dicecode']) ? $_GET['dicecode'] : "${times}d${max_side}";
            // Set parameters for Qithub API
            $params = [
                'is_mode_debug' => IS_MODE_DEBUG,
                'dicecode'      => $dicecode,
            ];
            // Request API
            $result_api = run_script('plugins/dice-roll', $params, false);
            $result     = decode_api_to_array($result_api);

            // Display result
            if (isset($result['result']) && 'OK' == $result['result']) {
                echo esc_html($result['value']);
            }

            if (IS_MODE_DEBUG) {
                echo_on_debug($params, 'Params to request');
                echo_on_debug($result, 'Result responce');
            }
            break;

        default:
            echo 'その他の処理';
    }
}

// END of MAIN
