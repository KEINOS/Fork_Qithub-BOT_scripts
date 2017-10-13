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
define('LOAD_DATA_EMPTY', false);

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
        // GitHub からの WebHook 処理
        case 'github':
            // WebHook からのデータ保存キー（データID）
            $id_data = 'WebHook-GitHub';

            // 保存済みデータの読み込み
            $log_data = load_data($id_data);
            if ($log_data === LOAD_DATA_EMPTY) {
                $log_data = array();
            }

            // ログの操作（view, delete）
            if (isset($_GET['method'])) {
                switch ($_GET['method']) {
                    // ログの一部削除（ログのキー）
                    case 'delete':
                        // 削除するログのキーを取得
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
                            echo "id ありません";
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

            // ログ（WebHook からのデータ）の保存
            } else {
                // GitHub からの POST データ（WebHook 内容）の追加保存
                $timestamp = date("Ymd-His");
                $log_data[$timestamp] = [
                    'getallheaders' => getallheaders(),
                    'get'           => $_GET,
                    'post'          => $_POST,
                    'ip'            => $_SERVER["REMOTE_ADDR"],
                    'host'          => gethostbyaddr($_SERVER["REMOTE_ADDR"]),
                    'raw_post_data' => file_get_contents('php://input'),
                ];
                $result = save_data($id_data, $log_data);
                if ($result==true) {
                    echo "Data saved." . PHP_EOL;
                    echo "Data ID/key was: ${id_data}/${timestamp}" . PHP_EOL;
                }
            }

            break;

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
                print_r($result);
            }

            break;

        case 'toot-daily':
            // 今日の日付のIDを取得
            $id_today = (int) date('Ymd');

            // トゥートIDの保存キー（データID）
            $id_data = 'toot_id_and_date_of_daily_toot';

            // 今日の初回トゥートのIDを取得
            $info_toot = load_data($id_data);

            // 初回トゥートが ある 場合の処理
            if ( $info_toot !== LOAD_DATA_EMPTY ) {
                echo "has pre toot";

            // 初回トゥートが ない 場合の処理
            } else {
                // トゥート内容の作成
                $date_today = date('Y/m/d')
                $msg = "${date_today} のトゥートを始めるよ！";

                // トゥート！
                $result_toot = post_toot([
                    'status'       => $result['value'],
                    'domain'       => $keys_api['domain'],
                    'access_token' => $keys_api['access_token'],
                    'visibility'   => 'unlisted',
                ]);
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
        debug_msg("【読み込みエラー】Data ID：'${id_data}' でのデータ読み込み時にエラーが発生しました。<br>\n【エラー内容】${result['value']}");
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

    return  ($result['result'] == 'OK');
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
    環境／その他 Functions
   --------------------------------- */
function debug_msg($str)
{
    if (IS_MODE_DEBUG) {
        $line = debug_backtrace()[1]['line'];
        //$line = print_r(debug_backtrace(),true);
        trigger_error(
            "${str}【呼び出し元】 ${line}行<br>\n",
            E_USER_WARNING
        );
    }
}

/**
 *  実行環境の言語を日本語に設定しタイムゾーンを設定する
 *
 * @param  string  $timezone デフォルトは東京
 * @return
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
