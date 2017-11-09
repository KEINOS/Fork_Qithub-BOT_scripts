<?php error_reporting(E_ALL);

/**
 * Front to the 「Qithub」 application.
 *
 * This application is a BOT that operate of, by and for the Qiita users.
 * See the link below for details.
 *
 * @copyright Qithub Organization All Rights Reserved
 * @license 2017/10/06 under discussion
 * @author https://github.com/Qithub-BOT/scripts/graphs/contributors
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
set_env_utf8_ja('Asia/Tokyo');

// DEV/DPYの環境設定・APIトークンなどを設定
// 引数に設定ファイル（'qithub.conf.json'）までのパスを指定してください。
// 設定方法は'./_samples/README.md'を参照してください。
set_env_file('../../qithub.conf.json');

// 本スクリプトの実行環境を設定
// DEV = DEVELOP（開発）環境, DPY = DEPLOY（本稼働）環境
set_env_as(DEV);

// 'system' および 'plugin' で使えるサーバー側のプログラム言語（CLI）
$extension_types = [
        'php'=>'.php',
        'python' =>'.py',
    ];

/* =====================================================================
    メイン
   =====================================================================
    定例処理（IS_PROC_REGULAR）
        クエリのパラメーターのないリクエスト。主にバックグラウンドで処理
        を行うタイプの処理。
    随時／臨時処理
        クエリにパラメーターがあるリクエスト
        基本パラメーター
        '&process='     ：'system'や'plugins'を組み合わせた複雑な処理
        '&plugin_name=' ：'plugins'を直接起動するシンプルな処理
        オプション・パラメーター
        '&mode=debug'   ： デバッグモード（is_mode_debug() = true）で動作
   ===================================================================== */

if (IS_PROC_REGULAR) {
    echo "定例処理を行う予定（in progress）";
    /** @todo 最新Qiita記事の取得 */
    /** @todo BOTのフォロワーの更新 */
    /** @todo Qiitaのフォロワーの更新 */
} else {
    switch (strtolower($_GET['process'])) {
        // 'github'
        // -------------------------------------------------------------
        // GitHub からの WebHook の受け取りテスト（内容の保存と確認）
        // クエリの'method'オプションによって保存データの閲覧・削除可能
        case 'github':
            include_once('./includes/github.php.inc');
            break;

        // 'sample'
        // -------------------------------------------------------------
        // BOT のトリガーテスト（プロセス）の動作サンプル
        case 'sample':
            include_once('./includes/sample.php.inc');
            break;

        // 'say-hello-world'
        // -------------------------------------------------------------
        // 'plugins/say-hello-world' を利用したサンプル
        case 'say-hello-world':
            include_once('./includes/say-hello-world.php.inc');
            break;

        // 'get-qiita-new-items'
        // -------------------------------------------------------------
        // Qiita記事の新着N件を表示するサンプル
        case 'get-qiita-new-items':
            include_once('./includes/get-qiita-new-items.php.inc');
            break;

        // 'toot-daily'
        // -------------------------------------------------------------
        // 日付ごとのスレッドでトゥートするサンプル。定例処理用のプロト
        // タイプ。
        case 'toot-daily':
            include_once('./includes/toot-daily.php.inc');
            break;

        // 'toot-daily-qiita-items'
        // -------------------------------------------------------------
        // 日付ごとのスレッドで新着Qiita記事をトゥートするサンプル
        case 'toot-daily-qiita-items':
            include_once('./includes/toot-daily-qiita-items.php.inc');
            break;

        // 'get-mastodon-user-info'
        // -------------------------------------------------------------
        // マストドンのユーザーアカウントおよびフォロワーの情報を表示する
        //
        // クエリの引数オプション
        //   '&use_cash=false' ：新規取得（更新）
        //   '&id=USER_ID'     ：指定ユーザーおよびフォロワーの情報を表示
        case 'get-mastodon-user-info':
            include_once('./includes/get-mastodon-user-info.php.inc');
            break;

        // 'roll-dice'
        // -------------------------------------------------------------
        // サイコロを振った結果と合計を表示するプラグイン（ PR #38）の
        // 動作テスト。
        //
        // クエリの引数オプション
        //   '&times='     ：振る回数
        //   '&max='       ：サイコロの最大出目
        //   '&dice_code=' ：ダイスコード
        //   '&mode=debug' ：デバッグモード（詳細表示）
        case 'roll-dice':
            include_once('./includes/roll-dice.php.inc');
            break;

        default:
            echo 'その他の処理';
    }
}

// END of MAIN
