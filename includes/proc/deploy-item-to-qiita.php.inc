<?php

// Set parameters for Qithub API
$params['is_mode_debug']  = is_mode_debug();
$params['is_env_dev']     = is_env_dev();
$params['access_token']   = ACCESS_TOKEN_QIITA;
$params['max_tag_number'] = 5;

if(! empty($_GET['id']) && isset($_GET['id'])){
    $params['id'] = $_GET['id'];
}

// Request API
$result_api = run_script('system/deploy-item-to-qiita', $params, false);

// Show result
// デバッグ・モード時は詳細を表示
if( is_mode_debug()){

    echo_on_debug($result_api, 'Result raw');

    $result = decode_api_to_array($result_api);

    if('OK' == $result['result']){
        $result['value'] = json_decode($result['value'], JSON_OBJECT_AS_ARRAY);    
        echo_on_debug(' result のステータスが OK の場合、 value 値は Qiita API の結果である JSON で返されるため配列に変換して表示。', 'Result decoded');    
    }
    echo_on_debug($result);    

}else{
    echo decode_api_to_json( $result_api );
}

die;
