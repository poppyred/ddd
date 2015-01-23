<?php
class BaseAction extends Action {
	
	public $user;
	public function _initialize(){					
		$user = $this->saveCurrentUserSession();
		if (empty($user)) {
			$actionName = strtolower(ACTION_NAME);
		if (!in_array($actionName, array("login", "checklogin"))) {
			header("Location: ".__APP__."/Index/login");
		exit;
		} 
	}
	$this->user = $user;
	}
	function saveCurrentUserSession() {
		return isset($_SESSION['user']) ? $_SESSION['user'] :null;
	}
	function _empty(){ 
		header("HTTP/1.0 404 Not Found");//使HTTP返回404状态码 
		$this->display("Public:404"); 
	 }
}
