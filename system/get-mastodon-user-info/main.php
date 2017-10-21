<?php
/* =====================================================================
 *  マストドンのユーザー情報を返します
 *
 *
 *
 *【このスクリプトの Qithub API パラメーター】
 *
 * ■受け取り必須項目
 *   必須項目
 *     'domain'       => string  トゥート先のドメイン名(ホスト名のみ)
 *     'access_token' => string  トゥートに必要なアクセストークン
 *   オプション項目
 *     'id'           => integer この項目があると、その値のユーザーIDの
 *                               情報を返します。指定されていない場合は、
 *                               access_tokenを作成した認証されたユーザ
 *                               情報が返されます。（つまりBOTのID）
 *     'use_cash'     => boolean キャッシュ（取得＆保存済みの）情報を利
 *                               用する（= true）か、新たに取得（= false）
 *                               するか指定します。
 *
 * ■出力項目
 *     'result' => string 'OK','NG'
 *     'value'  => json   'account'キーと'followers'キーの値に各々Mastodon
 *                        からのレスポンス内容をJSONのまま入れて返します。
 *
   ===================================================================== */

/* =====================================================================
    Main
   ===================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 定数
define('DIR_NAME_DATA', 'data');
define('DIR_SEP', DIRECTORY_SEPARATOR);
define('YES', true);
define('NO', false);

// 標準入力を取得
$arg = get_api_input_as_array();

/**
 * リクエストにより受け取ったパラメーターが必須項目を満たしていた場合、
 * BOT もしくはオプションで指定されたユーザーの Mastodon アカウント情報
 * およびフォロワーの情報を取得しレスポンスとして返します。
 */
if (is_requirement_complied($arg)) {
    // 必須項目
    $access_token = $arg['access_token'];
    $domain       = $arg['domain'];

    // デフォルト値
    $use_cash        = YES;
    $is_self_account = YES;

    // キャッシュの利用確認
    if (isset($arg['use_cash'])) {
        $use_cash = ($arg['use_cash'] == true);
    }

    // データやキャッシュの保存先ディレクトリのパス
    $path_dir_current = dirname(__FILE__) . DIR_SEP;
    $path_dir_data    = $path_dir_current . DIR_NAME_DATA . DIR_SEP;

    // BOT 自身もしくは指定ユーザーの Mastodon アカウント情報の取得に必要
    // なエンドポイント設定とキャッシュのファイルパス設定
    if (isset($arg['id']) && ! empty($arg['id'])) {
        // ユーザーが指定されている場合の設定
        $is_self_account = NO;
        $id_target       = (int) $arg['id'];
        $path_file_cache = $path_dir_data . "${id_target}.dat";
        $endpoint        = "/api/v1/accounts/${id_target}";
    } else {
        // BOT の場合の処理
        // BOT のアカウントID が取得済みであれば、そのIDのキャッシュの
        // ファイルパスを設定し、自身(BOT)のエンドポイントを設定
        $is_self_account = YES;
        $path_file_data_botid = $path_dir_data . 'id_self.dat';
        if (file_exists($path_file_data_botid)) {
            $id_target       = unserialize(file_get_contents($path_file_data_botid));
            $path_file_cache = $path_dir_data . "${id_target}.dat";
        } else {
            // BOT のアカウントIDを保存するためキャッシュをオフ（更新）
            $use_cash = NO;
        }
        $endpoint = '/api/v1/accounts/verify_credentials';
    }

    // レスポンス用の JSON データ（$result_json）取得
    if (( $use_cash == YES ) && ( file_exists($path_file_cache) )) {
        // キャッシュの読み込み
        $data_loaded = unserialize(file_get_contents($path_file_cache));
        // レスポンス用データ（キャッシュ有効か否かを付加）
        $result_json = json_encode(['use_cash'=>$use_cash] + $data_loaded);
    } else {
       //$use_cash=YES でもキャッシュファイルが無い場合があるので再セット
        $use_cash = NO;

       // アカウント情報を取得
        $query  = 'curl -X GET';
        $query .= " --header 'Authorization: Bearer ${access_token}'";
        $query .= " -sS https://${domain}${endpoint};";
        $result_json_info = `$query`;

        // フォロワー情報を取得
        $result_array_info = json_decode($result_json_info, JSON_OBJECT_AS_ARRAY);
        $id_target = $result_array_info['id'];
        $endpoint  = "/api/v1/accounts/${id_target}/followers";
        $query  = 'curl -X GET';
        $query .= " --header 'Authorization: Bearer ${access_token}'";
        $query .= " -sS https://${domain}${endpoint};";
        $result_json_followers  = `$query`;
        $result_array_followers = json_decode($result_json_followers, JSON_OBJECT_AS_ARRAY);

        // 保存するデータ
        $data_to_save = [
            'updated_at'      => time(),
            'id_target'       => $id_target,
            'is_self_account' => $is_self_account,
            'account'         => $result_array_info,
            'followers'       => $result_array_followers,
        ];

        // BOT のアカウントIDを保存（初回のみ）
        if ($is_self_account && ! file_exists($path_file_data_botid)) {
            file_put_contents($path_file_data_bot, serialize($id_target));
        }

        // 取得したアカウントの情報を保存（キャッシュ）
        $path_file_cache = $path_dir_data . "${id_target}.dat";
        file_put_contents($path_file_cache, serialize($data_to_save));

        // レスポンス用データ（キャッシュ有効か否かを付加）
        $result_json = json_encode(['use_cash'=>$use_cash] + $data_to_save);
    }

    // リクエストの結果
    /** @todo サーバが500の場合なども'OK'を返してしまうので要改善 */
    $result = 'OK';
    echo encode_array_to_api([
        'result' => $result,
        'value'  => $result_json,
    ]);
} else {
    // 必須項目が足りない場合のレスポンス処理
    $temp = print_r($arg, true);
    echo encode_array_to_api([
        'result' => 'NG',
        'value'  => "必須項目が足りません。\n${temp}",
    ]);
}


die();

/* ---------------------------------------------------------------------
    Functions
   --------------------------------------------------------------------- */

/**
 *  トゥートに最低限必要な項目を網羅しているかチェックします
 *
 * @param  array   $arg   API から渡された配列
 * @return boolean        必須項目を満たしていた場合は true
 */
function is_requirement_complied($arg)
{
    return isset($arg['domain']) && isset($arg['access_token']);
}


/* ---------------------------------------------------------------------
    Getter Functions
   --------------------------------------------------------------------- */
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

function get_api_input_as_array()
{
    return json_decode(get_api_input_as_json(), JSON_OBJECT_AS_ARRAY);
}

function get_api_input_as_json()
{
    return urldecode(get_stdin_first_arg());
}

/**
 * get_stdin_first_arg function.
 *
 * CLIからの標準入力を取得。必須条件の第一引数を満たしていない場合は、余
 * 計な処理を避ける為、強制終了。
 *
 * @access public
 * @return void
 */
function get_stdin_first_arg()
{
    global $argv;

    // 引数１は必須
    if (empty($argv[1])) {
        /* @SuppressWarnings */
        print_r($argv);
        /* @SuppressWarnings */
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
 * 日本語環境を大前提としたシステムなので、マルチバイト対応していない場
 * は余計な処理をさせないために強制終了。
 * 対応している場合は、日本語（UTF-8）の設定と、タイムゾーンを設定する。
 * （デフォルトタイムゾーン=東京, 日本）
 *
 * @access public
 * @param string $timezone (default: 'Asia/Tokyo')
 * @return void
 * @SuppressWarnings
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

function dir_exists($path_dir)
{
    return is_dir($path_dir);
}
