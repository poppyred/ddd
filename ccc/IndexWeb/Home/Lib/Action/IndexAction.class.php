<?php
// 本类由系统自动生成，仅供测试用途
class IndexAction extends Action {

    Public function _initialize(){
        require_once(APP_PATH.'Common/common.php');

        if( !empty($_COOKIE['email']) && !empty($_COOKIE['password']) && empty($_SESSION['user']) && empty($_SESSION['id']) ){
            $email = $_COOKIE['email'];
            $password = $_COOKIE['password'];

            $result = real_login($email, $password);

            if($result['result'] === 'success'){
                $_SESSION['user'] = $email;
                $_SESSION['id'] = $result['id'];
                $_SESSION['is_reverse'] = $result['is_reverse'];
            }
        }
    }

    public function index(){
		$this->display();
    }

    public function login(){
    	$email = $_POST['email'];
    	$password = $_POST['password'];

    	/*$url = 'http://121.201.11.7/client/Index/login';
    	$data = "type=web&username=$email&password=$password";
    	$result = http_post_data($url, $data);*/

    	$result = real_login($email, md5($password));

        if($result['result'] === 'success'){
            $_SESSION['user'] = $email;
            $_SESSION['id'] = $result['id'];
            $_SESSION['is_reverse'] = $result['is_reverse'];

            if(!empty($_POST['remember'])){
                setcookie('email', $email, time() + 3600 * 24 * 30, '/');
                setcookie('password', md5($password), time() + 3600 * 24 * 30, '/');
            }else{
                setcookie('email', '', time() - 3600);
                setcookie('password', '', time() - 3600);
            }
        }

        $data = array(
            'status' => $result['result']
        );

    	print_r(json_encode($data));
    }

}