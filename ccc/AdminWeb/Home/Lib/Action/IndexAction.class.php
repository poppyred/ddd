<?php
// 本类由系统自动生成，仅供测试用途
class IndexAction extends Action {
	public function index(){
		if(!empty($_SESSION['admin'])){
			header("Location: ".__APP__."/Clientmgr/");
		}else{
			header("Location: ".__APP__."/Index/login");
		}
	}
	public function login(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			unset($_SESSION['admin']);
			unset($_SESSION['id']);
			unset($_SESSION['level']);
			$this->display();
		}
		else{
			//var_dump($_POST["username"]);var_dump($_POST["password"]);die;
			if(isset($_POST['username']) && isset($_POST['password'])){
					$cnd["user"] = trim($_POST['username']);
					$cnd["pwd"] = trim($_POST['password']); //md5()
					$list = M("admin");
					$rslt = $list->where($cnd)->find();
					//print_r($list->getLastSQL());exit;
					if(!empty($rslt)){
						$_SESSION['admin'] = $rslt["user"];
						$_SESSION['id'] = $rslt["id"];
						$_SESSION['level'] = $rslt["level"];
						/*if($rslt["level"]==1){
							header("Location: ".__APP__."/Clientmgr/");
						}else{
						*/
						header("Location: ".__APP__."/FlowChart/");
						//}
					} else {
						//$this->success("用户名或密码输入错误，请核对后再试！",U("/Index/login"));
						//header("Location: ".__APP__."/Index/login");
						$this->assign('error', "用户名或密码错误，请核对后再试！");
						$this->display();
					}
			}
		}
	}
	public function logout(){
		unset($_SESSION['admin']);
		unset($_SESSION['id']);
		unset($_SESSION['level']);
		$this->redirect('Index/login');
	}
}
