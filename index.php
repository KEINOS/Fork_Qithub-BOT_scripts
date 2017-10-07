<?php
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
    Main
   ===================================================================== */
/* ---------------------------------
    初期設定
   --------------------------------- */

// 言語設定->日本語 地域->東京 にセット
set_utf8_ja('Asia/Tokyo');

// 定数設定エリア
define('IS_MODE_DEBUG', $_GET['mode'] == 'debug');
define('IS_PROC_REGULAR', ! isset($_GET['process'])); // 定例処理
define('IS_PROC_DEMAND', isset($_GET['process']));    // 随時処理
define('DIR_SEP', DIRECTORY_SEPARATOR);

// 各種グローバル変数の設定エリア
$extension_types = [
        'php'=>'.php',
        'python' =>'.py',
    ];

/* ---------------------------------
    各種外部APIに必要なキーの取得
   --------------------------------- */
$keys_api = get_api_keys('../../qithub.conf.json', 'qiitadon');


/* ---------------------------------
    定例処理
   --------------------------------- */
if( IS_PROC_REGULAR ){
    echo "定例処理を行う予定（in progress）";
    // 最新Qiita記事の取得
} else {
/* ---------------------------------
    随時処理
    クエリの'process'値で分岐処理
   --------------------------------- */
    switch( strtolower($_GET['process']) ){
        case 'github':
            echo 'GitHubからのWebHook処理';
            break;
        case 'say-hello':
            echo '\'say-hello\'プラグインの実行';
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
 * @param  string  $script_name     実行
 * @param  json    $params          スクリプトに渡すJSONオブジェクト
 * @param  boolean $run_background  trueの場合、実行結果を待たずにバック
 *                                  グラウンドで実行します
 * @return json or boolean          バックグラウンド実行の場合は
 */
function run_script($script_name, $params, $run_background = true)
{
    $lang_type = get_lang_type('say-hello');
    $command   = get_cli_command(
        $lang_type,
        $script_name,
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
 *  引数の配列をJSON+URLエンコードします
 *
 *  'system'および'plugins'は、このデータ形式で入力を受け付けます。
 *
 * @param  array $lang_script スクリプトの実行言語（'php','python'）
 * @return string             エンコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function array_urlencoded_json($array_value)
{
    $array['is_mode_debug'] = IS_MODE_DEBUG;
    $array['values'] = $array_value;

    $json = json_encode($array_value);
    $json = urlencode($json);

    return "${json}";
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
 * @param  string $name_dir
 * @return string プログラム言語名。false の場合は引数に問題があるか定義
 *                されていない拡張子が"main.xxxx"に使われています。
 */
function get_lang_type($name_dir)
{
    global $extension_types;

    $path_basic = "./${name_dir}/main";

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
 * @param  string $lang_script スクリプトの実行言語（'php','python'）
 * @param  string $name_script CLIで実行するスクリプトのパス
 * @param  string $array_value CLIでスクリプトに渡す引数の配列データ
 * @return string              CLI用のコマンド
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function get_cli_command($lang_script, $name_script, $array_value)
{
    $path_cli    = get_path_exe($lang_script);
    $path_script = get_path_script($id_issue, $name_script);
    $argument    = array_urlencoded_json($array_value);
    $command     = "${path_cli} ${path_script} ${argument}";

    return  $command;
}


function get_path_script($id_issue, $name_script)
{
    $path_dir_scripts = realpath('./_scripts/') . DIR_SEP . $id_issue;
    $path_file_script = $path_dir_scripts . DIR_SEP . $name_script;

    if (! file_exists($path_file_script)) {
        throw new Exception("不正なファイルパスの指定 ${path_file_script}");
        $path_file_script = "./unknown_path";
    }

    return $path_file_script;
}

/**
 *  スクリプトを CLI で実行する際に必要なプログラム言語のパスを取得する
 *
 * @param  string $lang_script スクリプトの実行言語（'php','python'）
 * @return string              スクリプト言語のパス
 */
function get_path_exe($lang_script)
{
    $lang_script = strtolower($lang_script);
    switch ($lang_script) {
        case 'php':
            $path_cli = '/usr/bin/php'; // PHP7 .0.22 cli
            break;
        default:
            throw new Exception("不明なプログラム言語の指定 ${lang_script}");
            $path_cli = null;
            break;
    }

    return $path_cli;
}

/* ---------------------------------
    環境／その他 Functions
   --------------------------------- */
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
