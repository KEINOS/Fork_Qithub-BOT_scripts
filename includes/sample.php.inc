<?php
// プロセス 'sample'
// ---------------------------------------------------------------------
// BOT のトリガーテスト（プロセス）の動作サンプル
//
// 基本スクリプトでデータ保存・読み込みを行う。データの保存自体は
// 'system/data-io' をラッパー関数で利用する例。

// サンプルデータのデータID
$id_data = 'sample';

// サンプルデータの作成と保存
$sample_data = [
    'time_stamp' => date("Y/m/d H:i:s"),
    'hoge'       => 'hoge',
];
save_data($id_data, $sample_data);

// サンプルデータの読み込み
$result = load_data($id_data);
print_r($result);