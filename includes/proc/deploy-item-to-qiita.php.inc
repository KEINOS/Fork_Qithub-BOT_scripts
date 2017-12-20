<?php

// Set parameters for Qithub API
$params['is_mode_debug']  = is_mode_debug();
$params['is_env_dev']     = is_env_dev();
$params['access_token']   = ACCESS_TOKEN_QIITA;
$params['max_tag_number'] = 5;

if(! empty($_GET['id']) && isset($_GET['id'])){
    $params['id'] = $_GET['id'];
}

//include('./system/deploy-item-to-qiita/functions.php.inc');

// Request API
$result_api = run_script('system/deploy-item-to-qiita', $params, false);

if( is_mode_debug()){
    $result = decode_api_to_array($result_api);

    echo_on_debug($result_api, 'Result raw');
    echo_on_debug($result, 'Result decoded');    
}else{
    echo decode_api_to_json( $result_api );
}

die;
