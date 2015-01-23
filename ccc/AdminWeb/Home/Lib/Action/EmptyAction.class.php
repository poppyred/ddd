<?php
// 本类由系统自动生成，仅供测试用途
class EmptyAction extends Action{ 
    function _empty(){ 
        header("HTTP/1.0 404 Not Found");//使HTTP返回404状态码 
        $this->display("Public:404"); 
    }
}    
