<?php

	function real_login($username, $password){
        $url = 'http://www.eflydns.com/client/Index/login';

        $param = array(
            'type' => 'web',
            'username' => $username,
            'password' => $password
        );

        $opt = array(
            'http' => array(
                'method' => 'POST',
                'header' => 'Content-type: application/x-www-form-urlencoded',
                'content' => http_build_query($param)
            )
        );

        $ctx = stream_context_create($opt);

        $result = file_get_contents($url, false, $ctx);
        $result = json_decode($result, true);

        return $result;
    }

?>