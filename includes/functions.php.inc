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

/* ---------------------------------------------------------------------
    Plugin Functions
    'plugins/xxxxxx'のQithub API ラッパー
   --------------------------------------------------------------------- */
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

    if (($result['result'] !== 'OK') && IS_MODE_DEBUG) {
        echo '<h1>hoge</h1><pre>';
        print_r($result_api);
        echo '</pre>';
    }

    return  ( $result['result'] == 'OK' ) ? $result : false;
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
    if (IS_MODE_DEBUG) {
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
    if (IS_MODE_DEBUG) {
        $title      = esc_html($title);
        $expression = esc_html($expression);

        if (! empty($title)) {
            $title = "<h1>${title}</h1>";
        }

        if (is_string($expression)) {
            echo "<div>${title}<pre>${expression}</pre></div>" . PHP_EOL;
        } else {
            echo '<div><pre>';
            print_r($expression);
            echo "</pre></div>" . PHP_EOL;
        }
    }
}

/**
 * set_utf8_ja function.
 *
 * 実行環境の言語を日本語に設定しタイムゾーンを設定する
 *
 * @access public
 * @param string $timezone (default: 'Asia/Tokyo')
 * @return void
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
