<?php
/* =====================================================================
    /index.php で使われる関数やラッパー
   ===================================================================== */

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
 * @return string                   実行結果を返します。URL エンコードさ
 *                                  れたJSONデータ文字列（Qithub API準拠）
 */
function run_script($dir_name, $params, $run_background = true)
{
    $lang_type = get_lang_type($dir_name);
    $command   = get_cli_command(
        $lang_type,
        $dir_name,
        $params
    );

    // NOTE: Be careful that commands are NOT single quoted
    //       but grave accent aka backquote/backtick.
    if ($run_background) {
        $log  = `$command > /dev/null &`;
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
 * Qithub 内の`system`および`plugins`からの出力結果の API データ・フォー
 * マット（URLエンコードされたJSON文字列）を PHP配列にデコードします。
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
 * @SuppressWarnings(PHPMD)
 */
function set_env_file($path_file_conf)
{
    if (file_exists($path_file_conf)) {
        $conf_json  = file_get_contents(trim($path_file_conf));
        $conf_array = json_decode($conf_json, JSON_OBJECT_AS_ARRAY);

        if (isset($conf_array['environment']['is_env_dev'])) {
            $is_env_dev = $conf_array['environment']['is_env_dev'];
        }

        if ('yes' == $is_env_dev) {
            $mastodon = $conf_array['mastodon_api']['access_token_dev'];
            $qiita    = $conf_array['qiita_api']['access_token_dev'];
            $github   = $conf_array['github_api']['access_token_dev'];
            defined('ACCESS_TOKEN_MASTODON') or define('ACCESS_TOKEN_MASTODON', $mastodon);
            defined('ACCESS_TOKEN_QIITA') or define('ACCESS_TOKEN_QIITA', $qiita);
            defined('ACCESS_TOKEN_GITHUB') or define('ACCESS_TOKEN_GITHUB', $github);
            defined('IS_SET_ENV') or define('IS_SET_ENV', true);
        } else {
            defined('IS_SET_ENV') or define('IS_SET_ENV', false);
        }
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
function set_env_as($arg){
    switch($arg){
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
 * 指定された 'system' および 'pugins' の スクリプトの実行言語を取得しま
 * す。第１引数に渡されたディレクトリ名より "main.xxx" の拡張子を調べ、
 * 該当する言語を返します。
 *
 * @access public
 * @param  string $dir_name  スクリプトのディレクトリ名
 * @return mixed             プログラム言語名。false の場合は引数に問題
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
 * 'system' および 'plugins' 内の実行スクリプト（main.???まで）の絶対パス
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
 * get_path_exe function.
 *
 * `systems` および `plugins` 内の各々のスクリプトを CLI で実行する際に
 * 必要なプログラム言語のパスを取得する
 *
 * @access public
 * @param  string $lang_type スクリプトの実行言語（'php','python',etc）
 * @return string            スクリプト言語のパス
 */
function get_path_exe($lang_type)
{
    $lang_type = strtolower($lang_type);
    switch ($lang_type) {
        case 'php':
            $path_cli = '/usr/bin/php'; // PHP 7.0.22 cli
            break;
        default:
            throw new Exception("不明なプログラム言語の指定 ${lang_type}");
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
 * デバッグなどでデータを表示する際のエスケープを行う。
 *
 * @access public
 * @param mixed $string
 * @return void
 */
function esc_html($string)
{
    $string = (string) $string;

    if (0 === strlen($string)) {
        return '';
    }

    $string = htmlspecialchars($string, ENT_QUOTES, 'UTF-8');

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
 * 問題なし： IS_VALID_HASH（ture）
 * 問題あり： IS_INVALID_HASH（false）
 * 主にアクセストークンの利用前のフォーマットのチェック用で有効なアクセ
 * ストークン（実際に使えるか否）かは確認しない。
 *
 * @access public
 * @param  mixed  $value_hash
 * @param  int    $len_hash (default: 64)
 * @return bool   IS_INVALID_HASH, IS_VALID_HASH
 */
function isValid_hash($value_hash, $len_hash = 64)
{
    if (strlen($value_hash) == $len_hash && ctype_xdigit($value_hash)) {
        return IS_VALID_HASH;
    } else {
        return IS_INVALID_HASH;
    }
}

/**
 * sanitize_id_user_qiita function.
 *
 * ユーザーの QiitaID が QiitadonID と同じであるとは限らないため、
 * Qiitadonの禁則文字の置換処理を行う。
 *
 * @access public
 * @param mixed $string
 * @return void
 * @todo QiitaID と QiitadonID が違う人がいるので置き換えが必要。
 * @note BOTのフォロワー情報から QiitaID は取得できる
 */
function sanitize_id_user_qiita($string)
{
    $string = trim($string);

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
    $string = str_replace('.', '_', $string);
    $string = str_replace('-', '_', $string);

    return $string;
};


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
 *    関数内で使う場合は`$with = "with";`を利用前に代入すること
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
 * デバッグモード（IS_MODE_DEBUG = true）時のみ、try, catchできるエラー
 * （E_USER_WARNING）出力関数。エラー発生元の取得も行う。
 *
 * @access public
 * @param mixed $string
 * @return void
 */
function debug_msg($string)
{
    if (is_mode_debug()) {
        $with   = "with";
        $string = esc_html($string);
        $line   = debug_backtrace()[1]['line'];
        trigger_error(
            "${string}【呼び出し元】 ${line}行{$with(BR_EOL)}",
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
