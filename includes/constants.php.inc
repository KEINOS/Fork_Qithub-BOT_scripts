<?php
/* =====================================================================
    /index.php で使われる定数
   ===================================================================== */

define('IS_MODE_DEBUG', isset($_GET['mode']) and ($_GET['mode'] == 'debug'));
define('IS_PROCESS', isset($_GET['process']) and ! empty($_GET['process'])); // 複合処理
define('IS_PLUGIN', isset($_GET['plugin_name']) and ! empty($_GET['plugin_name'])); // プラグイン単体処理
define('IS_REGULAR_JOB', ! IS_PROCESS and ! IS_PLUGIN); // バックグラウンド定例処理
define('DIR_SEP', DIRECTORY_SEPARATOR);
define('BR_EOL', '<br>' . PHP_EOL);
define('LOAD_DATA_EMPTY', false);
define('SAVE_DATA_SUCCESS', true);
define('SAVE_DATA_FAIL', false);
define('TOOT_SUCCESS', true);
define('TOOT_FAIL', false);
define('IS_VALID_HASH', true);
define('IS_INVALID_HASH', false);
define('DEV', 'develop');
define('DPY', 'deploy');

// 以下の定数は'functions.php.inc'の`set_env_file()`で定義しています
// ACCESS_TOKEN_MASTODON
// ACCESS_TOKEN_QIITA
// ACCESS_TOKEN_GITHUB
// IS_SET_ENV
//
// 以下の定数は'functions.php.inc'の`set_env_as()`で定義しています
// IS_ENV_DEV
