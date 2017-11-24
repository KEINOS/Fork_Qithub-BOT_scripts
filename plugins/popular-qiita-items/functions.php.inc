<?php
/* =====================================================================
    プラグイン：popular-qiita-items 用ユーザー関数

    - 環境設定系
        - set_env_utf8_ja()

    - 標準入力系
        - get_api_input_as_array()
        - get_api_input_as_json()
        - get_stdin_first_arg()

    - データ操作系
        - load()
        - save()
        - delete()

    - 文字列操作系
        - get_num_fron_id()

    - その他
        - dir_exists()
   ===================================================================== */

/* ---------------------------------------------------------------------
    環境設定系 ユーザー関数
   --------------------------------------------------------------------- */

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
    標準入力系 ユーザー関数
   --------------------------------------------------------------------- */

/**
 * get_api_input_as_array function.
 * 標準入力を PHP 配列で取得
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
 * 標準入力を JSON 配列で取得
 *
 * @access public
 * @return void
 */
function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

/**
 * get_stdin_first_arg function.
 * Qiita API に準拠した標準入力の第１引数を(RAWで)取得
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
        $msg_api = [
            'result' => 'NG',
            'value'  => 'Argument is empty.'
        ];
        $msg_enc = json_encode($msg_api);
        $msg_enc = urlencode($msg_enc);
        die($msg_enc);
    }

    return trim($argv[1]);
}

/* ---------------------------------------------------------------------
    データ操作系 ユーザー関数

    'sqlite'などを利用しないのは現サーバからの引っ越しで環境依存させない
    ため。負荷によって要検討。
   --------------------------------------------------------------------- */

/**
 * save function.
 *
 *
 * @access public
 * @param  string $id
 * @param  mixed  $data
 * @return bool
 */
function save($id, $data)
{
    $id = trim((string) $id);
    $path_file_data = PATH_DIR_DATA . $id . EXT_FILE_DATA;

    // データディレクトリのチェック
    if (! dir_exists(PATH_DIR_DATA)) {
        if (mkdir(PATH_DIR_DATA, 0705)) {
            touch(PATH_DIR_DATA . 'index.html');
            file_put_contents(
                PATH_DIR_DATA . '.gitignore',
                '*'.EXT_FILE_DATA
            );
            file_put_contents(
                PATH_DIR_DATA . '.htaccess',
                "Options -ExecCGI\norder deny,allow\ndeny from all"
            );
        }
    }
    // データの保存
    $data   = serialize($data);
    $result = (file_put_contents($path_file_data, $data)) ? true : false;

    return $result;
}

/**
 * load function.
 *
 * @access public
 * @param  string $id
 * @return mixed
 */
function load($id)
{
    $id = trim((string) $id);
    $path_file_data = PATH_DIR_DATA . $id . EXT_FILE_DATA;

    if (file_exists($path_file_data)) {
        $contents = file_get_contents($path_file_data);
        $contents = unserialize($contents);

        return $contents;
    } else {
        return IS_DATA_UNINITIALIZED;
    }
}

/**
 * delete function.
 *
 * @access public
 * @param  string $id
 * @return bool
 */
function delete($id)
{
    $id = trim((string) $id);
    $path_file_data = PATH_DIR_DATA . $id . EXT_FILE_DATA;

    return (file_exists($path_file_data) && unlink($path_file_data));
}

/* ---------------------------------------------------------------------
    文字列操作系 ユーザー関数
   --------------------------------------------------------------------- */

/**
 * get_num_fron_id function.
 *
 * アイテムIDから'PublicArticle/'以降の数値のみを取り出す
 *
 * @access public
 * @param  string  $string
 * @return integer
 */
function get_num_fron_id($string)
{
    if (preg_match('/[0-9]+$/', $string, $result)) {
        return $result[0];
    }
}

/* ---------------------------------------------------------------------
    その他 ユーザー関数
   --------------------------------------------------------------------- */

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
