<?php

/* ---------------------------------------------------------------------
    Validation Functions
   --------------------------------------------------------------------- */

/**
 * is_requirement_complied function.
 *
 * トゥートに最低限必要な項目を網羅しているかチェックします
 *
 * @access public
 * @param  array   $arg   API から渡された配列
 * @return bool           必須項目を満たしていた場合は true
 * @url https://qiita.com/mpyw/items/51bf42fe1ad8a7e535aa#isset%E3%81%AE%E7%89%B9%E5%BE%B4
 */
function is_requirement_complied($arg)
{
    return isset(
        $arg['domain'],
        $arg['access_token'],
        $arg['status'],
        $arg['visibility']
    );
}


/* ---------------------------------------------------------------------
    Getter Functions
   --------------------------------------------------------------------- */

/**
 * encode_array_to_api function.
 *
 * 配列を Qithub API（JSON + URLエンコード文字列）にエンコードします
 * 'system'および'plugins'は、このデータ形式で入力を受け付けます。
 *
 * @param  array  $array_value エンコードするPHP配列
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
 * get_api_input_as_array function.
 *
 * 標準入力から受け取った Qithub API データをPHP配列として受け取ります。
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
 *
 * 標準入力から受け取った Qithub API データをJSON文字列として受け取ります。
 *
 * @access public
 * @return string
 */
function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

/**
 * get_stdin_first_arg function.
 *
 * 標準入力から受け取った第１引数を Qithub API データとして受け取ります。
 *
 * @access public
 * @return void
 * @SuppressWarnings(PHPMD)
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

/* ---------------------------------------------------------------------
    Miscellaneous Functions
   ------------------------------------------------------------------ */

/**
 * set_utf8_ja function.
 *
 * Set language to Japanese UTF-8 and Time zone to Japan
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

/**
 * dir_exists function.
 *
 * `is_dir()`の語感用関数
 *
 * @access public
 * @param  mixed  $path_dir
 * @return string
 */
function dir_exists($path_dir)
{
    return is_dir($path_dir);
}
