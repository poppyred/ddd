<?php
// 本类由系统自动生成，仅供测试用途
class HelpAction extends Action {

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

    public function help(){
		$this->display();
    }

    public function tools(){
        $domain = !empty($_GET['domain']) ? $_GET['domain'] : '';

        $this->assign('domain', $domain);
        $this->display();
    }

    public function get_whois_info(){
        $domain = $_GET['domain'];

        $url = 'http://121.201.12.61/script/eflydns_whois.php';

        $data = array(
            'domain' => $domain
        );

        $opt = array(
            'http' => array(
                'method' => 'POST',
                'header' => 'Content-type: application/x-www-form-urlencoded',
                'content' => http_build_query($data)
            )
        );

        $ctx = stream_context_create($opt);

        echo file_get_contents($url, false, $ctx);
    }
}