<?php
/* ---------------------------------------------------------------------
    Getter Functions
   ------------------------------------------------------------------ */
function get_api_input_as_array()
{
    return json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
}

function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

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
 * Set language to Japanese UTF-8 and Time zone to Japan
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
 * Decode and curiculating dice code
 */
function roll_dice($dice_code)
{
    $param = explode('D', strtoupper(trim($dice_code)));
    $sum   = 0;
    $roll  = array();
    $times = intval($param[0]);
    $max   = intval($param[1]);

    for ($i = 0; $i < $times; $i++) {
        $roll[] = rand(1, $max);
        $sum += end($roll);
    }

    // $roll = implode(" ", $roll);
    $result = [
        'roll' => $roll,
        'sum'  => $sum,
    ];

    return $result;
}
