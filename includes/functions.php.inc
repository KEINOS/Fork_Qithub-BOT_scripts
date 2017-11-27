<?php
/* =====================================================================
    /index.php で使われる関数やラッパー
   ===================================================================== */

/**
 * call_plugin function.
 *
 * `plugins`の単独実行用関数。引数に通常は $_GET の内容が渡される。
 * 引数の配列で渡されるキー（下記以外はプラグイン依存）
 *
 *    必須
 *        plugin_name => string : プラグイン名。プラグインディレクトリ名。
 *
 *    オプション
 *        pg_lang => string : 実行するプログラム言語の拡張子（ドットなし）。
 *                            プラグイン内に複数の`main.xxx`が存在する場
 *                            合に指定。指定されていない場合は、本体スク
 *                            リプト 'index.php' の $extension_types で
 *                            指定されている順番で最初にマッチしたもの。
 *        mode => string   ： 'debug'の場合は、本体スクリプトはデバッグ
 *                            モードで動作している。
 * クエリのリクエスト例
 *    https://sample.com/qithub/?plugin_name=say-hello-world
 *    https://sample.com/qithub/?plugin_name=say-hello-world&pg_lang=go
 *    https://sample.com/qithub/?plugin_name=say-hello-world&mode=debug
 *    https://sample.com/qithub/?plugin_name=say-hello-world&pg_lang=go&mode=debug
 *
 * @access public
 * @param  array  $query
 * @param  bool   $run_background trueの場合は
 * @return array
 */
function call_plugin($query, $run_background = false)
{
    $isset_name = (isset($query['plugin_name']) && ! empty($query['plugin_name']));
    $isset_lang = (isset($query['pg_lang']) && ! empty($query['pg_lang']));
    if ($isset_name) {
        $plugin_name = (string) $query['plugin_name'];
        $plugin_dir  = "plugins/${plugin_name}";
        $lang        = ($isset_lang) ? (string) $query['pg_lang'] : '';
        $result      = run_script(
            $plugin_dir,
            $_GET,
            $run_background,
            $lang
        );

        return $result;
    }
}

/**
 * run_script function.
 *
 * Qithub API に準拠した 'system' および 'plugin' のスクリプトを実行（指
 * 定されたエンドポイント先の`main.xxx`を実行）します。
 *
 * 実行に必要なパラメーターは各スクリプトのディレクトリにある README.md
 * を参照して準拠してください。
 *
 * @access public
 * @param  string  $dir_name        スクリプトのディレクトリ名（エンドポ
 *                                  イント名）
 *                                  'system/<script name>'
 *                                  'plugin/<script name>'
 * @param  array   $params          スクリプトに渡す配列（パラメーター）
 * @param  bool    $run_background (default: true)
 *                                  バッグクランド実行オプション。trueを
 *                                  指定した場合、実行結果を待たずにバッ
 *                                  クグラウンドで実行します
 * @param  string  $pg_lang(default: '')
 *                                  実行言語指定オプション。複数プログラ
 *                                  ム言語がスクリプト・ディレクトリ内に
 *                                  ある場合、実行する言語の拡張子を指定。
 * @return string                   実行結果を返します。URL エンコードさ
 *                                  れたJSONデータ文字列（Qithub API準拠）
 */
function run_script($dir_name, $params, $run_background = true, $lang = '')
{
    $lang_type = get_lang_type($dir_name, $lang);

    if ($lang_type==false) {
        $lang     = esc_html($lang);
        $dir_name = esc_html($dir_name);
        die("<b>Error:</b> Can't determine language type: '${lang}' in '${dir_name}'.");
    }

    $command   = get_cli_command(
        $lang_type,
        $dir_name,
        $params
    );

    // NOTE: Be careful that commands are NOT single quoted
    //       but grave accent aka backquote/backtick.
    if ($run_background) {
        $log = `$command > /dev/null &`;
        $array_value = [
            'result' => 'OK',
            'value'  => "Running background.\n ${log}",
        ];
        $json_raw = json_encode($array_value);
        $result   = urlencode($json_raw);
    } else {
        $result = `$command`;
    }

    return $result;
}

/**
 * decode_api_to_array function.
 *
 * Qithub 内の `system` および `plugins` からの出力結果の API データ・
 * フォーマット（URLエンコードされたJSON文字列）を PHP配列にデコードしま
 * す。
 *
 * @access public
 * @param  string  $json_enc APIからのURLエンコード済みJSON
 * @return array             デコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function decode_api_to_array($json_enc)
{
    $json_raw = urldecode($json_enc);
    $array    = json_decode($json_raw, JSON_OBJECT_AS_ARRAY);

    return $array;
}

/**
 * encode_array_to_api function.
 *
 * 配列を Qithub API データフォーマット（JSON & URLエンコード）にエンコー
 * ドします
 *
 * 'system'および'plugins'は、このデータ形式で入力を受け付けます。
 *
 * @access public
 * @param  array  $array_value スクリプトの実行言語（'php','python'）
 * @return string              エンコード結果
 * @link https://github.com/Qithub-BOT/scripts/issues/16
 */
function encode_array_to_api($array_value)
{
    $array['is_mode_debug'] = is_mode_debug();
    $array['values']        = $array_value;

    $json_raw = json_encode($array_value);
    $json_enc = urlencode($json_raw);

    return $json_enc;
}

/* ---------------------------------------------------------------------
    セッター系 Functions
   --------------------------------------------------------------------- */

/**
 * set_env_file function.
 *
 * 環境設定やAPIのトークンを定数に設定します。引数で指定されたJSONファイ
 * ルを読み込んで内容を設定します。
 *
 * @access public
 * @param  string $path_file_conf  Path to environment config file.
 * @return void
 * @uses   Use 'set_env_as()' before, in order to make work 'is_env_dev()'.
 * @SuppressWarnings(PHPMD)
 */
function set_env_file($path_file_conf)
{
    if (file_exists($path_file_conf)) {
        $conf_json  = file_get_contents(trim($path_file_conf));
        $conf_array = json_decode($conf_json, JSON_OBJECT_AS_ARRAY);

        if (is_env_dev()) {
            $mastodon = $conf_array['mastodon_api']['access_token_dev'];
            $qiita    = $conf_array['qiita_api']['access_token_dev'];
            $github   = $conf_array['github_api']['access_token_dev'];
            defined('IS_SET_ENV') or define('IS_SET_ENV', true);
        } else {
            $mastodon = $conf_array['mastodon_api']['access_token'];
            $qiita    = $conf_array['qiita_api']['access_token'];
            $github   = $conf_array['github_api']['access_token'];
            defined('IS_SET_ENV') or define('IS_SET_ENV', false);
        }
        defined('ACCESS_TOKEN_MASTODON') or define('ACCESS_TOKEN_MASTODON', $mastodon);
        defined('ACCESS_TOKEN_QIITA') or define('ACCESS_TOKEN_QIITA', $qiita);
        defined('ACCESS_TOKEN_GITHUB') or define('ACCESS_TOKEN_GITHUB', $github);
    } else {
        $path_file = esc_html($path_file);
        trigger_error(
            "confファイルが存在しません。${path_file}",
            E_USER_WARNING
        );
        die();
    }
}

/**
 * set_env_as function.
 *
 * 実行環境を DEVELOP（開発）か DEPLOY（本稼働）かを設定する。
 *
 * @access public
 * @param  string $arg  DEV or DPY の定数で指定
 * @return void
 * @SuppressWarnings(PHPMD)
 */
function set_env_as($arg)
{
    switch ($arg) {
        case ( DEV === $arg ):
            defined('IS_ENV_DEV') or define('IS_ENV_DEV', true);
            break;
        case ( DPY === $arg ):
            defined('IS_ENV_DEV') or define('IS_ENV_DEV', false);
            break;
        default:
            $arg = esc_html($arg);
            trigger_error(
                "不正な環境設定値が渡されました。（${arg}）",
                E_USER_WARNING
            );
            die;
    }
}

/**
 * set_env_utf8_ja function.
 *
 * 実行環境の言語を日本語に設定しタイムゾーンを設定する
 *
 * @access public
 * @param string $timezone (default: 'Asia/Tokyo')
 * @return void
 * @SuppressWarnings(PHPMD)
 */
function set_env_utf8_ja($timezone = 'Asia/Tokyo')
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

/* ---------------------------------------------------------------------
    ゲッター系 Functions
   --------------------------------------------------------------------- */
/**
 * get_lang_type function.
 *
 * 'system' および 'plugins' で実行に必要なプログラム言語名を返します。
 * 第１引数に渡されたディレクトリ名より "main.xxx" の拡張子を調べ、該当
 * するプログラム言語を返します。第２引数で拡張子が指定してあった場合、
 * 利用可能であれば、同様に該当するプログラム言語を返します。
 *
 * ex) $result = get_lang_type('plugins/sample');
 *
 * @access public
 * @param  string $dir_name  スクリプトのディレクトリ名
 * @param  string $needle    優先して利用したい言語の拡張子。指定がない
 *                           場合は`$extension_types`に最初にヒットした
 *                           言語。
 * @return mixed             プログラム言語名。false の場合は引数に問題
 *                           があるか定義されていない拡張子が"main.xxxx"
 *                           に使われています。
 */
function get_lang_type($dir_name, $needle = '')
{
    global $extension_types; //システムの利用可能プログラム言語

    $needle        = trim((string) $needle);
    $list_ext_main = get_lang_list_main($dir_name);

    if (empty($list_ext_main)) {
        debug_msg("There's no 'main.xxx' file at '${dir_name}'.");
        return false;
    }

    if (! empty($needle) && ! empty($list_ext_main)) {
        $ext_in_ext_types = array_search(".${needle}", $extension_types);
        $ext_in_list_main = in_array($needle, $list_ext_main);
        if ($ext_in_ext_types && $ext_in_list_main) {
            $result_lang = $ext_in_ext_types;
        } else {
            debug_msg("Can't determine extension type. File not exist at '${dir_name}'.");
            $result_lang = false;
        }
    } else {
        $result_lang   = false; //念のための初期化
        $list_ext_main = array_flip($list_ext_main);

        foreach ($extension_types as $lang => $ext) {
            $ext = trim($ext, '.');
            if (array_key_exists($ext, $list_ext_main)) {
                $result_lang = $lang;
                break;
            }
        }

        if ($result_lang === false) {
            debug_msg("Can't determine extension type. File not exist at '${dir_name}'.");
        }
    }

    return $result_lang;
}

/**
 * get_lang_list_main function.
 *
 * 'plugins' / 'system' が実行できるプログラム言語の一覧を取得します。
 * ディレクトリ内の"main.xxx" の拡張子一覧を返します。
 *
 * @access public
 * @param  string $dir_name 'plugins/xxxxxxxx', 'system/xxxxxxxx'形式
 * @return array
 */
function get_lang_list_main($dir_name)
{
    $dir_name = (string) $dir_name;
    $dir_path = '.' . DIR_SEP .$dir_name;

    if (dir_exists($dir_path)) {
        $main_files = glob($dir_path . DIR_SEP .'main*');

        //拡張子一覧を返す
        return filter_var(
            $main_files,
            FILTER_CALLBACK,
            [ "options" => function ($path) {
                    return pathinfo($path, PATHINFO_EXTENSION);
            }]
        );
    } else {
        $dir_name = esc_html($dir_name);
        debug_msg("Error: Can not find plugin at '${dir_name}'.");

        return array();
    }
}

/**
 * get_api_keys function.
 *
 * 各種API用に必要な各種キーの取得をします
 *
 * 第１引数に渡されたパスにあるJSON形式のCONFファイルの中から第２引数の
 * キーを持つ配列の値を返します。
 *
 * @access public
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
 * のコマンドを生成します
 *
 * @param  string $lang_type   スクリプトの実行言語（'php','python'など）
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
 * get_path_script function.
 *
 * 'system' および 'plugins' 内の実行スクリプト（main.xxxまで）の絶対パス
 * を返します。
 *
 * @access public
 * @param  string $dir_name    スクリプトのディレクトリ
 *                             'system/<script name>'
 *                             'plugin/<script name>'
 * @param  string $lang_type   スクリプトの実行言語（'php','python',etc）
 * @return string              スクリプトの絶対パス
 */
function get_path_script($dir_name, $lang_type)
{
    global $extension_types;

    /* does lang type exists */

    $ext = $extension_types[$lang_type];

    $path_dir_scripts = '.' . DIR_SEP .$dir_name . DIR_SEP;
    $path_file_script = $path_dir_scripts . 'main' . $ext;

    if (! file_exists($path_file_script)) {
        debug_msg("不正なファイルパスの指定 ${path_file_script}");
        $path_file_script = "./unknown_path";
    } else {
        $path_file_script = realpath($path_file_script);
    }

    return $path_file_script;
}

/**
 * get_path_exe function.
 *
 * `systems` および `plugins` 内の各々のスクリプトを CLI で実行する際に
 * 必要なプログラム言語のパスを取得する
 *
 * @access public
 * @param  string $lang_type スクリプトの実行言語（'php','python',etc）
 * @return string            スクリプト言語のパス
 * @ref: https://www.xrea.com/spec/ サーバーの利用可能CGI/SSI
 */
function get_path_exe($lang_type)
{
    $lang_type = strtolower($lang_type);
    switch ($lang_type) {
        case 'php':
            $path_cli = '/usr/local/bin/phpcli'; // php 7.0.22 cli
            break;
        case 'python':
            $path_cli = '/usr/bin/python'; // python 2.7.5
            break;
        case 'ruby':
            $path_cli = '/usr/bin/ruby'; // ruby 2.3.0p0
            break;
        case 'perl':
            $path_cli = '/usr/bin/perl'; // perl 5 v16
            break;
        case 'go':
            $path_cli = '/usr/bin/go run'; // go1.6.3
            break;
        default:
            debug_msg("不明なプログラム言語の指定 ${lang_type}");
            $path_cli = null;
            break;
    }

    return $path_cli;
}

/* ---------------------------------------------------------------------
    Wrapper Functions

    'system/xxxxxx' や 'plugins/xxxxxx' の Qithub API ラッパー
   --------------------------------------------------------------------- */
/**
 * delete_toot function.
 *
 * 第１引数の配列で指定されたトゥートIDとトークンで過去のトゥートを削除
 * します。
 *
 * 使用例＆必須項目）
 *   $is_toot_deleted = delete_toot([
 *      'domain'       => $keys_api['domain'],
 *      'access_token' => $keys_api['access_token'],
 *      'id'           => $id_toot,
 *   ]);
 *
 * @access public
 * @param  array  $params
 * @return bool
 */
function delete_toot($params)
{
    $dir_name_api   = 'system/delete-toot';
    $run_background = false;

    $result_api = run_script($dir_name_api, $params, $run_background);
    $result     = decode_api_to_array($result_api);

    return  ( $result['result'] == 'OK' );
}

/**
 * post_toot function.
 *
 * 第１引数の配列で指定されたトークンとメッセージ内容でトゥートを実行し
 * ます。第１引数の配列内の'visibility'キーの値は Mastodon に準拠。
 *
 * 使用例＆必須項目）
 *   $result_toot = post_toot([
 *       'status'       => 'Hello world',
 *       'domain'       => $keys_api['domain'],
 *       'access_token' => $keys_api['access_token'],
 *       'visibility'   => 'unlisted',
 *   ]);
 *
 * @access public
 * @param  array  $params
 * @return mixed           Mastodon API からのトゥート結果。失敗した場合
 *                         は TOOT_FAIL(false) が返される。
 */
function post_toot($params)
{
    $dir_name_api   = 'system/post-toot';
    $run_background = false;

    $result_api = run_script($dir_name_api, $params, $run_background);
    $result     = decode_api_to_array($result_api);

    if (($result['result'] !== 'OK') && is_mode_debug()) {
        echo_on_debug($result_api, 'TOOT FAILED: Posted toot info');
    }

    return  ( $result['result'] == 'OK' ) ? $result : TOOT_FAIL;
}


/* ---------------------------------------------------------------------
    DATA I/O Functions
    'system/data-io'の Qithub API ラッパー
   --------------------------------------------------------------------- */
/**
 * load_data function.
 *
 * 指定されたキーの保存データの読み込みをします
 *
 * @access public
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
 * save_data function.
 *
 * 指定されたキーで、データの保存をします
 *
 * @access public
 * @param  string  $id_data 保存したいデータのキー
 * @param  mixed   $data    保存したいデータ（変数,配列,オブジェクト）
 * @return bool             データの保存成功は true、失敗は false
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
 * delete_data function.
 *
 * 指定されたキーのデータを削除をします
 *
 * @access public
 * @param  string  $id_data 保存したデータのキー
 * @return bool             削除されれば true
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


/* ---------------------------------------------------------------------
    サニタイズ（正規化）／バリデーション Functions
   --------------------------------------------------------------------- */

/**
 * esc_html function.
 *
 * 引数の文字列をデバッグなどでデータを表示する際のエスケープに使う。
 *
 * @access public
 * @param  string $string
 * @return void
 */
function esc_html($string)
{
    $string = (string) $string;

    if (0 === strlen($string)) {
        return '';
    }

    //$string = htmlspecialchars($string, ENT_QUOTES, 'UTF-8');
    $string = htmlentities($string, ENT_QUOTES, 'UTF-8');
    $string = str_replace('\\', '&#092;', $string);

    return $string;
}

/**
 * sanitize_array_to_dump function.
 * 配列をダンプ表示するためのエスケープ処理
 *
 * @access public
 * @param  array  $array
 * @return void
 */
function sanitize_array_to_dump($array)
{
    return filter_var(
        $array,
        FILTER_CALLBACK,
        ["options" => function ($value) {
            return esc_html($value);
        }]
    );
}

/**
 * sanitize_id_user_qiita function.
 *
 * ユーザーの QiitaID が QiitadonID と同じであるとは限らないため、
 * Qiitadonの禁則文字の置換処理を行う。
 *
 * @access public
 * @param  string $string
 * @return void
 * @todo QiitaID と QiitadonID が違う人がいるので置き換えが必要。
 * @note BOTのフォロワー情報から QiitaID は取得できる
 */
function sanitize_id_user_qiita($string)
{
    $string = trim((string) $string);

    $table = [
        'sample-user' => 'sample_user',
    ];
    $search  = array_keys($table);
    $replace = array_values($table);

    /* Common replacement */
    $string = str_replace($search, $replace, $string);
    $string = str_replace('@github', '', $string);
    $string = str_replace('@', 'at', $string);
    $string = str_replace('.', '_', $string);
    $string = str_replace('-', '_', $string);

    return $string;
};

/**
 * sanitize_tagname_qiitadon function.
 *
 * Qiitaタグを Mastodon のハッシュタグ用に正規化変換
 *
 * @access public
 * @param  string $string
 * @return void
 */
function sanitize_tag_for_qiitadon($string)
{
    $string = (string) $string;
    $string = mb_convert_kana($string, 'aKV');
    $string = trim($string);
    $string = str_replace('++', 'プラプラ', $string);
    $string = str_replace('+', 'プラス', $string);
    $string = str_replace('#', 'Sharp', $string);
    $string = str_replace('.', '', $string);
    $string = str_replace('-', '_', $string);

    return $string;
}


/**
 * isValid_url function.
 *
 * 引数のURLが有効なアドレスかを返す。
 * 主にQiitadonユーザーが存在するか（ユーザーページが存在するか）の確認
 * に利用。QiitadonAPI でチェックするにはオーバーヘッドが多く面倒なため。
 *
 * @access public
 * @param  mixed $sUrl
 * @return bool
 */
function isValid_url($sUrl)
{
    if (( $sResponce = @file_get_contents($sUrl) ) == false) {
        // HTTPステータスコードを取得する
        list($version, $status_code, $msg) = explode(' ', $http_response_header[0], 3);
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
 * isValid_hash function.
 *
 * 指定桁数の半角英数字であるかのチェック結果を返す。
 * 主にアクセストークンの利用前のフォーマットのチェック用で有効なアクセ
 * ストークン（実際に使えるか否）かは確認しない。
 *
 * @access public
 * @param  string $value_hash
 * @param  int    $len_hash (default: 64)
 * @return bool
 */
function isValid_hash($value_hash, $len_hash = 64)
{
    $value_hash = (string) $value_hash;
    $len_hash   = (integer) $len_hash;

    if (strlen($value_hash) == $len_hash && ctype_xdigit($value_hash)) {
        return IS_VALID_HASH;
    } else {
        return IS_INVALID_HASH;
    }
}


/**
 * is_tag_test function.
 *
 * 渡された文字列に「テスト」や「test」が含まれているか確認する。主に
 * Qiita記事のタグなどに「テスト」タグが含まれているかのチェックに使う。
 *
 * @access public
 * @param  string $tag
 * @return bool
 */
function is_tag_test($tag)
{
    $tag = (string) $tag;
    $tag = trim(strtolower($tag), '#');

    switch ($tag) {
        case 'test':
        case 'テスト':
        case 'てすと':
        case 'testing':
        case 'temp':
            $result = true;
            break;
        default:
            $result = false;
    }

    return $result;
}

/**
 * is_title_test function.
 *
 * 渡された文字列に「テスト」や「test」が含まれているか確認する。主に
 * Qiita記事のタイトルなどに「テスト」が含まれているかのチェックに使う。
 *
 * @access public
 * @param  string $title
 * @return bool
 */
function is_title_test($title)
{
    return is_tag_test($title);
}

/* ---------------------------------------------------------------------
    環境／デバッグ用／その他 Functions
   --------------------------------------------------------------------- */

/**
 * with function.
 *
 * 定数を文字列内で展開するための関数（Laravel風）
 *
 * 使用例：
 *    define('SAMPLE', 'sample');
 *    $s = "The value is {$with(SAMPLE)}.";
 * 注意：
 *    関数・クラス内で使う場合は`$with = "with";`を利用前に代入すること
 *
 * @access public
 * @param mixed $value
 * @return mixed
 */
function with($value)
{
    return $value;
}
$with = "with";

/**
 * is_mode_debug function.
 *
 * デバッグモードの定数が bool であるため false が指定された場合に備えて
 * 確実にチェックするための関数
 *
 * @access public
 * @return bool
 * @url http://php.net/manual/ja/function.defined.php#121433
 */
function is_mode_debug()
{
    return defined('IS_MODE_DEBUG') && IS_MODE_DEBUG;
}

/**
 * is_env_dev function.
 *
 * 実行環境が DEVELOP か DEPLOY かを返す。 DEVELOPの場合は true
 * 定数'IS_ENV_AS'の定義が false の場合に備えて、確実にチェックするための関数。
 *
 * @access public
 * @return bool
 */
function is_env_dev()
{
    return defined('IS_ENV_DEV') && (IS_ENV_DEV === true);
}

/**
 * debug_msg function.
 *
 * デバッグモード時にエラー発生行付きで任意の表示する関数。
 *
 * デバッグモード（IS_MODE_DEBUG = true）時のみ、try, catchできるエラー
 * （E_USER_WARNING）出力関数。エラー発生元の取得も行う。
 *
 * @access public
 * @param  string $string
 * @param  bool   $return この引数が true の場合には情報を返します。
 * @return void
 */
function debug_msg($string, $return = false)
{
    if (is_mode_debug()) {
        $with   = "with";
        $string = (string) $string;
        $string = esc_html($string);
        $line   = debug_backtrace()[1]['line'];
        $file   = debug_backtrace()[1]['function'];
        $string = "${string}【呼び出し元】 ${line}行目 '${file}()' {$with(BR_EOL)}";

        if ($return) {
            return $string;
        } else {
            trigger_error(
                $string,
                E_USER_WARNING
            );
        }
    }
}

/**
 * echo_on_debug function.
 *
 * echo/print_r を見やすく出力するデバッグ用関数。
 * デバッグモード（IS_MODE_DEBUG = true）時のみ動作。主にテスト動作で
 * 変数の内容をチェックする時などに利用する。
 *
 * @access public
 * @param  mixed  $expression
 * @param  string $title (default: '') 見出しを付けたい場合に指定
 * @return void
 * @SuppressWarnings(PHPMD)
 */
function echo_on_debug($expression, $title = '')
{
    if (is_mode_debug()) {
        if (! empty($title)) {
            $title = esc_html($title);
            $title = "<h1>${title}</h1>";
        }

        if (is_string($expression)) {
            $expression = esc_html($expression);
            echo "<div>${title}<pre>${expression}</pre></div>" . PHP_EOL;
        } else {
            $expression = esc_html(print_r($expression, true));
            echo "<div>${title}<pre>${expression}</pre></div>" . PHP_EOL;
        }
    }
}

/**
 * dir_exists function.
 *
 * is_dir()の語感互換関数
 *
 * @access public
 * @param  string $path_dir
 * @return bool
 */
function dir_exists($path_dir)
{
    return is_dir((string) $path_dir);
}
