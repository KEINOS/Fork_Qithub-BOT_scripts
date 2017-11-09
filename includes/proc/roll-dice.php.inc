<?php
//
// プロセス 'roll-dice'
// ---------------------------------------------------------------------
// サイコロを振った結果と合計を表示するプラグイン（ PR #38）の動作テスト。
//
// クエリの引数オプション
//   '&times='     ：振る回数
//   '&max='       ：サイコロの最大出目
//   '&dice_code=' ：ダイスコード
//   '&mode=debug' ：デバッグモード（詳細表示）

// Number of fling（サイコロを振る回数）
$times = '1';
if (isset($_GET['times']) && is_numeric($_GET['times']) && ! empty($_GET['times'])) {
    $times = (integer) $_GET['times'];
}
// Number of side of a dice（サイコロの最大出目）
$max_side = '6';
if (isset($_GET['max']) && is_numeric($_GET['max']) && ! empty($_GET['max'])) {
    $max_side = intval($_GET['max']);
}
// Get result as emoji（サイコロの出目を emoji）
$use_emoji = 'no';
if (isset($_GET['use_emoji']) && ($_GET['use_emoji']=='yes')) {
    $use_emoji = 'yes';
}
// Set 'dice_code'
$dice_code = isset($_GET['dice_code']) ? $_GET['dice_code'] : "${times}d${max_side}";
// Set parameters for Qithub API
$params = [
    'is_mode_debug' => IS_MODE_DEBUG,
    'dice_code'     => $dice_code,
    'use_emoji'     => $use_emoji,
];
// Request API
$result_api = run_script('plugins/roll-dice', $params, false);
$result     = decode_api_to_array($result_api);

// Display result
if (isset($result['result']) && 'OK' == $result['result']) {
    $roll = implode(' ', $result['value']['roll']);
    $sum  = $result['value']['sum'];
    echo esc_html("Result: ${roll} Sum: ${sum}");
}

if (IS_MODE_DEBUG) {
    echo_on_debug($params, 'Params to request');
    echo_on_debug($result, 'Result responce');
}
