<?php

/* ---------------------------------------------------------------------
    Qiita API Parser Functions
   ------------------------------------------------------------------ */
/**
 * patch_to_qiita function.
 *
 * @access public
 * @param  string $id_qiita
 * @param  string $data_json
 * @param  string $access_token
 * @return string               Result of cURL execution.
 */
function patch_to_qiita($id_qiita, $data_json, $access_token)
{

    $id_qiita = (string) $id_qiita;
    $url_request = URL_BASE_QITHUB . $id_qiita;
    $curl   = curl_init($url_request);
    $option = [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_CUSTOMREQUEST  => 'PATCH',
        CURLOPT_POSTFIELDS     => $data_json,
        CURLOPT_HTTPHEADER     => [
            "Authorization: Bearer ${access_token}", //アクセストークン
            'Content-Type: application/json',
        ],
    ];

    //print_output_as_api('OK',$option);

    curl_setopt_array($curl, $option);
    $result = curl_exec($curl);
    curl_close($curl);

    return $result;
}

/**
 * post_to_qiita function.
 * 新規Qiita記事の投稿とその結果を返す。
 *
 * @access public
 * @param  string $data_json
 * @param  string $access_token
 * @return string
 */
function post_to_qiita($data_json, $access_token)
{
    $curl = curl_init('https://qiita.com/api/v2/items');
    $option = [
        CURLOPT_CUSTOMREQUEST => 'POST',
        CURLOPT_POSTFIELDS    => $data_json,
        CURLOPT_HTTPHEADER    => [
            "Authorization: Bearer ${access_token}", //アクセストークン
            'Content-Type: application/json',
        ],
        CURLOPT_RETURNTRANSFER => true,
    ];

    curl_setopt_array($curl, $option);
    $result = curl_exec($curl);
    curl_close($curl);

    return $result;
}

/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */

/**
 * get_api_input_as_array function.
 * 標準入力より Qiita API 準拠で受け取ったデータを配列で返す。
 *
 * @access public
 * @return array
 */
function get_api_input_as_array()
{
    return json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
}

/**
 * get_api_input_as_json function.
 * 標準入力より Qiita API 準拠で受け取ったデータをJSON文字列で返す。
 *
 * @access public
 * @return string JSON format
 */
function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

/**
 * get_id_timestamp function.
 * タイムスタンプをベースとした ID を取得する。
 *
 * @access public
 * @param  string $format (default: 'Ymd_His')
 * @return string
 */
function get_id_timestamp($format = 'Ymd_His')
{
    return (string) date($format);
}

/**
 * get_items_github_raw function.
 *
 * @access public
 * @param  string $url
 * @return string
 */
function get_item_github_raw($url)
{
    $url = (string) $url;

    if (( $contents = @file_get_contents($url) ) == false) {
        // HTTPステータスコードを取得する
        list($version, $status_code, $msg) = explode(' ', $http_response_header[0], 3);
        // ステータスコードごとの処理
        switch ($status_code) {
            case 404:
                print_output_as_api(
                    'NG',
                    "File not found while getting contents from:\n${url}.",
                    '404 Error'
                );
                return false;
                break;
            default:
                print_output_as_api(
                    'NG',
                    "Undefined error while getting contents from:\n${url}.",
                    "${status_code} Error"
                );
                return false;
                break;
        }
    } else {
        //正常にfile_get_contentsで取得できた場合の処理
        return $contents;
    }
}

/**
 * get_lines_first_and_second function.
 * 改行付き文字列の最初の行（タイトル）と３行目（タグ）を配列で返す。
 *
 * @access public
 * @param  string $string
 * @return array
 * @ref https://stackoverflow.com/questions/9097682/obtain-first-line-of-a-string-in-php
 */
function get_lines_first_and_second($string)
{
    $string   = (string) $string;
    $lines[0] = trim(strtok($string, "\n")); //Get 1st line
    $lines[1] = trim(strtok("\n"));          //Get 2nd line

    //Free memory
    unset($string);
    strtok('', '');

    return $lines;
}

/**
 * get_lines_rest function.
 * 記事のタイトルとタグ以外の残りの本文を返します。
 *
 * @access public
 * @param  string $string
 * @return string
 */
function get_lines_rest($string)
{
    $num_limit_explode = 4;
    $string = (string) $string;
    $result = explode("\n", $string, $num_limit_explode);
    $result = $result[$num_limit_explode-1];

    return trim($result);
}

/**
 * get_stdin_first_arg function.
 * 標準入力の第１引数返す（QithubAPI準拠）
 *
 * @access public
 * @return void
 */
function get_stdin_first_arg()
{
    // CLIからの標準入力を取得
    global $argv;

    // 引数１は必須
    if (empty($argv[1])) {
        print_r($argv);
        die("Argument is empty.");
    }

    return trim($argv[1]);
}

/**
 * get_tags_as_array function.
 * Qithub items フォーマットのタグ一覧を配列で返す。
 *
 * @access public
 * @param  string $string
 * @return array
 */
function get_tags_as_array($string)
{
    $string = (string) $string;
    $string = mb_convert_kana($string, 'aKV');
    $string = str_replace(['，', '、', '､'], ',', $string);
    $array  = explode(',', $string);
    foreach ($array as $key => $value) {
        $array[$key] = sanitize_tag_string($value);
    }

    return $array;
}
/* ---------------------------------------------------------------------
    エンコード・デコード系 Functions
   ------------------------------------------------------------------ */
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
    $json_raw = json_encode($array_value);
    $json_enc = urlencode($json_raw);

    return $json_enc;
}

/**
 * print_output_as_api function.
 *
 * 処理結果を Qithub の スクリプト間 API に準拠して標準出力する
 *
 * @access public
 * @param  string $status    "OK" or "NG" の文字列
 * @param  string $contents  処理結果
 * @return void
 */
function print_output_as_api($status, $contents)
{
    $status = trim((string) $status);
    $status = ( 'ok' == $status ) ? 'OK' : $status;

    $contents = (string) $contents;

    $array = [
        'result' => $status,
        'value'  => $contents,
    ];

    echo encode_array_to_api($array);
    die();
}

/* ---------------------------------------------------------------------
    Miscellaneous Functions
   ------------------------------------------------------------------ */
/**
 * create_url_github_raw function.
 *
 * @access public
 * @param  string $id_item
 * @return string
 */
function create_url_github_raw($id_item)
{
    $id_item = (string) $id_item;
    if (is_valid_id_item($id_item)) {
        $url = URL_BASE_GITHUB . $id_item . '.md';
        return $url;
    } else {
        return 'not a valid item id';
    }
}

/**
 * dir_exists function.
 * Alternate to `is_dir()` func to ease understanding.
 *
 * @access public
 * @param  mixed  $path_dir
 * @return string
 */
function dir_exists($path_dir)
{
    return is_dir($path_dir);
}

/**
 * format_tags_for_qiita function.
 *
 * @access public
 * @param  array $array
 * @return void
 */
function format_tags_for_qiita($array, $max_tag_number = '')
{
    $result = array();

    if (empty($max_tag_number)) {
        $max_tag_number = MAX_TAG_NUM_QIITA;
    }
    if (is_env_dev()) {
        $result[] = ['name' => 'test'];
    }

    foreach ($array as $value) {
        $result[] = ['name' => $value];
    }

    $result = array_slice($result, 0, $max_tag_number);

    return $result;
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
 * set_env_as_dev function.
 *
 * 実行環境を DEVELOP か否かを設定する
 *
 * @access public
 * @param  bool   $is_env_dev
 * @return bool
 */
function set_env_as_dev($is_env_dev)
{
    $is_env_dev = ($is_env_dev) ? true : false;
    if (defined('IS_ENV_DEV')) {
        print_output_as_api('NG', 'Already defined IS_ENV_DEV');
    }

    return define('IS_ENV_DEV', $is_env_dev);
}

/**
 * is_valid_id_item function.
 *
 * @access public
 * @param  string $id_item
 * @return bool
 */
function is_valid_id_item($id_item)
{
    return true;
}

/**
 * sanitize_content function.
 * 文字列（Qiitaコラボ記事）のサニタイズが必要な場合の処理
 *
 * @access public
 * @param  string $string
 * @return string
 */
function sanitize_content($string)
{
    $string = uniform_eol($string);
    $string = trim($string);

    return $string;
}

/**
 * sanitize_tag_string function.
 *
 * @access public
 * @param  string $string
 * @return string
 */
function sanitize_tag_string($string)
{
    $string = (string) $string;
    $string = trim($string);

    return $string;
}

/**
 * set_utf8_ja function.
 * Set language to Japanese UTF-8 and Time zone
 *
 * @access public
 * @param  string $timezone (default: 'Asia/Tokyo')
 * @return void
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

/**
 * uniform_eol function.
 * 文字列の改行コードを統一して返します。
 *
 * @access public
 * @param  string $string
 * @param  string $to (default: "\n")
 * @return string
 * @ref https://qiita.com/suin/items/cbbbe8844fd734fa20e2
 */
function uniform_eol($string, $to = "\n")
{
    return strtr($string, array(
        "\r\n" => $to,
        "\r" => $to,
        "\n" => $to,
    ));
}
