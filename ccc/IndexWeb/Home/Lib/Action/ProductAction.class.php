<?php
// 本类由系统自动生成，仅供测试用途
class ProductAction extends Action {

    Public function _initialize(){
        require_once(APP_PATH.'Common/common.php');

        if( !empty($_COOKIE['email']) && !empty($_COOKIE['password']) && empty($_SESSION['user']) && empty($_SESSION['id']) ){
            $email = $_COOKIE['email'];
            $password = $_COOKIE['password'];

            $result = real_login($email, $password);

            if($result['result'] === 'success'){
                $_SESSION['user'] = $email;
                $_SESSION['id'] = $result['id'];
            }
        }
    }

}