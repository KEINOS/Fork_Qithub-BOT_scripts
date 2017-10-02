<?php
/**
 * Front to the Qithub application.
 *
 * Call this script from the browser, WebHook, cron, etc. to execute
 * the BOT routine.
 *
 * See more at GitHub:
 *     https://github.com/Qithub-BOT/sctipts
 *
 */

/* =====================================================================
    Main
   ================================================================== */
// Set language to Japanese and Time zone to Japan
set_utf8_ja('Asia/Tokyo');

// Define constants
define('IS_MODE_DEBUG', $_GET['mode'] == 'debug');
define('DIR_SEP', DIRECTORY_SEPARATOR);

/* ---------------------------------
    Get info to use the APIs
   --------------------------------- */
$path_file_conf = '../../qithub.conf.json';
$name_conf      = 'qiitadon'; // Key name at 'qithub.conf.json'

$conf_json  = file_get_contents(trim($path_file_conf));
$conf_array = json_decode($conf_json, JSON_OBJECT_AS_ARRAY);
$keys_api   = $conf_array[$name_conf]; //Get API keys and host name.

/* ---------------------------------
    Run plugins
   --------------------------------- */

// (1) Toot 'Hello Wordl!'. See issue #10 at 'scripts' repo.
echo run_script($keys_api, 'issue10');


die();

/* =====================================================================
    Functions
   ================================================================== */

function run_script($keys_api, $id_issue)
{
    // Set plugin information to run script via CLI
    switch ($id_issue) {

        // 'Hello world!' tooter.
        // See './_scripts/issue10/README.md' for detail.
        case 'issue10':
            $lang_script = 'php';
            $name_script = 'hello_world.php';
            $command     = get_cli_command(
                $keys_api,
                $lang_script,
                $id_issue,
                $name_script
            );
            break;

        default:
            echo "Plugin '${id_issue}' does not exist at 'run_script()'";
            break;
    }

    // Run as background script unless debug mode.
    // NOTE: Be careful that commands are NOT single quoted
    //       but grave accent aka backquote/backtick.
    if (IS_MODE_DEBUG) {
        $result = `$command`;
    } else {
        $log    = `$command > /dev/null &`;
        $result = 'OK';
    }

    return $result;
}

/* ---------------------------------
    Getter Functions
   --------------------------------- */
function get_cli_command($keys_api, $lang_script, $id_issue, $name_script)
{
    $path_cli    = get_path_cli($lang_script);
    $path_script = get_path_script($id_issue, $name_script);
    $argument    = get_argument($keys_api);
    $command     = "${path_cli} ${path_script} ${argument}";

    return  $command;
}

function get_argument($keys_api)
{
    $array['is_mode_debug'] = IS_MODE_DEBUG;
    $array['keys'] = $keys_api;
    $array['get']  = $_GET;
    $array['post'] = $_POST;

    $json = json_encode($array);
    $json = urlencode($json);

    return "${json}";
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

function get_path_cli($lang_script)
{
    $lang_script = strtolower($lang_script);
    switch ($lang_script) {
        case 'php':
            $path_cli = '/usr/bin/php';
            break;
        default:
            throw new Exception("不明なプログラム言語の指定 ${lang_script}");
            $path_cli = null;
            break;
    }

    return $path_cli;
}

/* ---------------------------------
    Miscellaneous Functions
   --------------------------------- */

/**
 * Set language to Japanese UTF-8 and Time zone.
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
 * `is_dir` alternate function to avoid Linux file system returning FALSE
 * if the directory does not have +x (executable) set for the php process.
 */
function dir_exists($file)
{
    return ((fileperms("$file") & 0x4000) == 0x4000);
}
