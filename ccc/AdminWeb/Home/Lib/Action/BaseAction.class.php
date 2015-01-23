<?php
class BaseAction extends Action {
	
	public $user;
	public function _initialize(){
		$user = $this->saveCurrentUserSession();
		if (empty($user)) {
			$actionName = strtolower(ACTION_NAME);
			if (!in_array($actionName, array("login", "checklogin"))) {
				header("Location: ".__APP__."/Index/index");
				exit;
			}
		}	
		$url = __ACTION__;
		//监控权限
		if($_SESSION['level'] == 0 ){
			if(strstr($url,"/FlowChart") || strstr($url,"/System")){
			}else{
				header("Location: ".__APP__."/FlowChart");	
			}
		}
		//客户支撑权限
		if($_SESSION['level'] == 2 ){
			if(strstr($url,"/Clientmgr") || strstr($url,"/Domainmgr") || strstr($url,"/ReverseDomain")){
			}else{
				header("Location: ".__APP__."/Clientmgr");	
			}
		}
		$this->user = $user;
	}
	function saveCurrentUserSession() {
		return isset($_SESSION['admin']) ? $_SESSION['admin'] :null;
	}

	function _empty(){ 
		header("HTTP/1.0 404 Not Found");//使HTTP返回404状态码 
		$this->display("Public:404"); 
	}
}
