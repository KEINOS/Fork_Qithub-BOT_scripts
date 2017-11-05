<?php
/**
 * 指定したサイコロを振った結果を返すスクリプト
 *
 * @copyright Qithub Organization All Rights Reserved
 * @license 2017/11/05 under discussion
 * @author @hidao80 (@hidao@Qiitadon.com)
 * @link https://github.com/Qithub-BOT/
 * @php version >=5.5.9-1
 */

require("functions.php.inc");

/* =====================================================================
    Main
   ===================================================================== */

// UTF-8, TIMEZONEを日本仕様に設定
set_utf8_ja('Asia/Tokyo');

// 標準入力を取得
$arg = get_api_input_as_array();

// ダイスコードを取得
$dice_code = $arg['dice_code'];

// メッセージの作成（RAW）
$msg_raw = roll_dice($dice_code);

// API準拠の出力結果作成
$msg_api = [
    'result' => 'OK',
    'value'  => $msg_raw
];
$msg_enc = json_encode($msg_api);
$msg_enc = urlencode($msg_enc);

// プラグインの処理結果を出力
echo $msg_enc;

die();
