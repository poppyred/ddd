<?php
Vendor('PHPMailer.class#phpmailer');
// 本类由系统自动生成，仅供测试用途
class IndexAction extends Action {
	
	public function index(){
		if(!empty($_SESSION['user'])){
			header("Location: ".__APP__."/Domain/domainList");
		}else{
			header("Location: ".__APP__."/Index/login");
		}

	}

	public function login(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){	
			cookie('email',null);
			cookie('password',null);

			unset($_SESSION['user']);
			unset($_SESSION['id']);
			$this->display();
		}
		else{
			//var_dump($_POST["checkcode"]);var_dump($_SESSION["myverify"]);die;
			if(isset($_POST['username']) && isset($_POST['password']) && $_POST['type'] == 'web'){
				$cnd["mail"] = trim($_POST['username']);
				$cnd["pwd"] = trim($_POST['password']); //md5()
				$list = M("client");
				$rslt = $list->where($cnd)->find();
				//echo $list->getLastSQL();				
				if($rslt['status']==1){
					echo 'error';
				}else{
					if(!empty($rslt)){
					
						$last_login = strtotime($rslt["login_time"]);
						$data["login_time"] = date("Y-m-d H:i:s");
						$span = floor((strtotime($data["login_time"]) - $last_login) / 60); //min
						/*if($span >= 5){
							//$param = array("id"=>$rslt["id"]);
							//http_post(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php", $param);
							file_get_contents(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php?id=".$rslt["id"]);
						}*/
						
						$list->where($cnd)->data($data)->save();
						
						echo '{"result":"success","id":'.$rslt["id"].'}'; //,"session":"'.$_SESSION['user'].'"
											
					} else {
						echo 'error';
					}
				}
				
			}else{
				if(isset($_POST['username']) && isset($_POST['password']) && isset($_POST['checkcode'])){
					if(md5(trim($_POST['checkcode'])) != $_SESSION['myverify']){
						$this->assign('error', "验证码错误，请输入正确的验证码！");
						$this->assign('mail', $_POST['username']);
						$this->display();
					}else{
						$cnd["mail"] = trim($_POST['username']);
						$cnd["pwd"] = md5(trim($_POST['password'])); //md5()
						//print_r($cnd["mail"]);
						//print_r($cnd["pwd"]);
						//exit;
						$list = M("client");
						$rslt = $list->where($cnd)->find();
						if($rslt['status']==1){
							$this->assign('error', "您的账户为冻结状态，请联系管理员！");
							$this->assign('mail', $_POST['username']);
							$this->display();
						}else{
							if(!empty($rslt)){
								$_SESSION['user'] = $rslt["mail"];
								$_SESSION['id'] = $rslt["id"];
								
								$last_login = strtotime($rslt["login_time"]);
								$data["login_time"] = date("Y-m-d H:i:s");
								$span = floor((strtotime($data["login_time"]) - $last_login) / 60); //min
								//print_r($span);exit;
								//if($span >= 5){
									//$param = array("id"=>$rslt["id"]);
									//$ddd = file_get_contents(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php?id=".$rslt["id"]);
									//$ddd = http_post(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php", $param);
									//print_r($ddd);exit;
								//}
								
								$list->where($cnd)->data($data)->save();
								header("Location: ".__APP__."/Domain/domainList");
							} else {
								//$this->success("用户名或密码输入错误，请核对后再试！",U("/Index/login"));
								$this->assign('error', "用户名或密码错误，请核对后再试！");
								$this->assign('mail', $_POST['username']);
								$this->display();
							}
						}
					}
				}
			}
		}
	}
	public function mgrClient(){
		$client = M('client');
		$data['mail'] = $_GET['mail'];
		$data['pwd'] = $_GET['pwd'];
		$entity = $client->where($data)->find();
		
		if(!empty($entity)){
			$_SESSION['user'] = $data['mail'];
			$_SESSION['id'] = $entity['id'];
		}
		$callback = $_REQUEST['callback'];
		$result = array('success'=>1);
		$jsonData = json_encode($result);
		echo $callback . "(" . $jsonData . ")";
		
	}
	
	public function user(){
		if(!empty($_SESSION['user'])){
			$cnd["id"] = $_SESSION['id'];
			$list = M("client");
			$rslt = $list->where($cnd)->find();
			$this->assign('user', $rslt);
			$this->display();
		}else{
			header("Location: ".__APP__."/Index/login");
		}
	}
	public function setCompany(){
		if(isset($_POST['comp']) && isset($_POST['user']) ){
			$cnd["mail"] = trim($_POST['user']);
			$list = M("client");
			$rslt = $list->where($cnd)->find();
			if(empty($rslt)){
					$this->ajaxReturn('error', '该用户不存在！', 0);
			}else{
				$data['org_name'] = $_POST['comp'];
				$is_ok = $list->where($cnd)->data($data)->save();
				if($is_ok===false){
					$this->ajaxReturn('error', '企业名称修改失败！', 0);	
				}else{
					$this->ajaxReturn('success', '企业名称修改成功！', 1);
				}
			}
		}
	}
	public function setPhone(){
		if(isset($_POST['phone']) && isset($_POST['user']) ){
			$cnd["mail"] = trim($_POST['user']);
			$list = M("client");
			$rslt = $list->where($cnd)->find();
			if(empty($rslt)){
					$this->ajaxReturn('error', '该用户不存在！', 0);
			}else{
				$data['link_phone'] = $_POST['phone'];
				$is_ok = $list->where($cnd)->data($data)->save();
				if($is_ok===false){
					$this->ajaxReturn('error', '手机号码修改失败！', 0);	
				}else{
					$this->ajaxReturn('success', '手机号码修改成功！', 1);
				}
			}
		}
	}
	public function setMan(){
		if(isset($_POST['man']) && isset($_POST['user']) ){
			$cnd["mail"] = trim($_POST['user']);
			$list = M("client");
			$rslt = $list->where($cnd)->find();
			if(empty($rslt)){
					$this->ajaxReturn('error', '该用户不存在！', 0);
			}else{
				$data['link_man'] = $_POST['man'];
				$is_ok = $list->where($cnd)->data($data)->save();
				if($is_ok===false){
					$this->ajaxReturn('error', '联系人修改失败！', 0);	
				}else{
					$this->ajaxReturn('success', '联系人修改成功！', 1);
				}
			}
		}
	}
	public function setWeixin(){
		if(isset($_POST['weixin']) && isset($_POST['user']) ){
			$cnd["mail"] = trim($_POST['user']);
			$list = M("client");
			$rslt = $list->where($cnd)->find();
			if(empty($rslt)){
					$this->ajaxReturn('error', '该用户不存在！', 0);
			}else{
				$data['link_weixin'] = $_POST['weixin'];
				$is_ok = $list->where($cnd)->data($data)->save();
				if($is_ok===false){
					$this->ajaxReturn('error', '微信绑定失败！', 0);	
				}else{
					$this->ajaxReturn('success', '微信绑定成功！', 1);
				}
			}
		}
	}
	
	public function verify(){
		//导入Image类库
		import("ORG.Util.Image");
		Image::buildImageVerify(4,1,"png",60,30,"myverify"); //(length,mode,type,width,height,verifyName)
	}
	//个人注册
	public function register(){
		$client = M('client');
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			unset($_SESSION['user']);
			unset($_SESSION['id']);
			$this->display();
		}else{
			if(isset($_POST['mail']) && isset($_POST['pwd']) && isset($_POST['code'])){
				if(md5(trim($_POST['code'])) != $_SESSION['myverify']){
					$this->ajaxReturn('验证码错误，请输入正确的验证码！', 'error',0);
				}else{
					$data['type'] = 0;
					$data['mail'] = trim($_POST['mail']);
					$data['pwd'] = md5(trim($_POST['pwd']));
					$data['org_name'] = '';
					$data['org_num'] = '';
					$data["reg_time"] = date("Y-m-d H:i:s");
					$is_ok = $client->add($data);
					if($is_ok){
						$_SESSION['user'] = $data["mail"];
						$_SESSION['id'] = $is_ok;
						$this->ajaxReturn('个人用户注册成功，正在跳转到域名列表...', 'success',1);
					}else{
						$this->ajaxReturn('个人用户注册失败，请联系管理员', 'error',0);	
					}
				}
			}
		}
	}
	//公司注册
	public function cregister(){
		$client = M('client');
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			unset($_SESSION['user']);
			unset($_SESSION['id']);
			$this->display();
		}else{
			if(isset($_POST['orgName']) && isset($_POST['orgCode']) && isset($_POST['mail']) && isset($_POST['pwd']) && isset($_POST['code'])){
				if(md5(trim($_POST['code'])) != $_SESSION['myverify']){
					$this->ajaxReturn('验证码错误，请输入正确的验证码！', 'error',0);
				}else{
					$data['type'] = 1;
					$data['mail'] = trim($_POST['mail']);
					$data['pwd'] = md5(trim($_POST['pwd']));
					$data['org_name'] = trim($_POST['orgName']);
					$data['org_num'] = trim($_POST['orgCode']);
					$data["reg_time"] = date("Y-m-d H:i:s");
					$is_ok = $client->add($data);
					if($is_ok){
						$_SESSION['user'] = $data["mail"];
						$_SESSION['id'] = $is_ok;
						$this->ajaxReturn('企业用户注册成功，正在跳转到域名列表...', 'success',1);
					}else{
						$this->ajaxReturn('企业用户注册失败，请联系管理员', 'error',0);	
					}
				}
			}
		}
	}
	public function checkUser(){
		if(!empty($_POST['eamil'])){
			$client = M('client');
			$vo = $client->where('mail="'.trim($_POST['eamil']).'"')->select();
			if($vo){
				$this->ajaxReturn(0,'error',0);
			}else{
				$this->ajaxReturn(1,'success',1);
			}
		}
	}
	
	public function forget(){
		if($_SERVER['REQUEST_METHOD'] == 'GET'){
			$this->display();	
		}else{
			if(!empty($_POST['mail']) && !empty($_POST['code'])){
				if(trim($_POST['code']) != $_SESSION['num']){
					$this->ajaxReturn('验证码错误，请输入正确的验证码！', 'error',0);
				}else{
					//$client = M('client');
					//$pwd = md5(trim($_POST['pwd'])); //md5()
					//$ret = $client->where("mail='".$_POST['mail']."'")->find();
					$this->ajaxReturn(1, 'success',1);
					/*$is_ok = $client->where("mail='".$_POST['mail']."'")->setField('pwd',$pwd);
					if($is_ok){
						$_SESSION['user'] = $ret["mail"];
						$_SESSION['id'] = $ret['id'];
						$this->ajaxReturn("修改密码成功！3秒自动跳转域名管理页面", 'success',1);
					}else{
						$this->ajaxReturn('修改密码错误，请联系管理员！', 'error',0);
					}*/
				}
			}
		}
		
	}
	
	public function sendCode(){
		if(!empty($_POST['mail'])){
			$client = M('client');
			$ret = $client->where("mail='".$_POST['mail']."'")->find();
			if(empty($ret)){
				$this->ajaxReturn('账号不存在。','error',0);
			}
			$num = rand(100000,999999);
			$_SESSION['num'] = $num;
			$is_ok = $this->send_mail($_POST['mail'],'EflyDNS - 修改密码','尊敬的'.$_POST['mail'].'用户，您好，本次修改密码的随机验证码为'.$num);
			if($is_ok=='ok'){
				$this->ajaxReturn('验证码成功发送。','success',1);
			}else{
				$this->ajaxReturn('获取验证码失败，请联系管理员。','error',0);
			}
		}
	}
	
	
	//发送邮件
	public function send_mail($add,$title, $content){
        //echo $content;
        $mail = new PHPMailer(); //建立邮件发送类
        $address = $add;//接收邮件地址
        $mail->IsSMTP(); // 使用SMTP方式发送
        $mail->Host = "smtp.163.com"; // 您的企业邮局域名
        $mail->SMTPAuth = true; // 启用SMTP验证功能
        $mail->Username = "eflydns@163.com"; // 发送方邮件地址(请填写完整的email地址)
        $mail->Password = "wwweflydns"; //发送放邮件密码
        $mail->Port=25;
        $mail->From = "eflydns@163.com"; //发送方邮件地址
        $mail->FromName = "eflydns";
        $mail->AddAddress("$address", "$address");//收件人地址，可以替换成任何想要接收邮件的email信箱,格式是AddAddress("收件人email","收件人姓名")
        //$mail->AddReplyTo("", "");
 
        //$mail->AddAttachment("/var/tmp/file.tar.gz"); // 添加附件
        //$mail->IsHTML(true); // set email format to HTML //是否使用HTML格式
        $mail->CharSet = "utf-8";
        $mail->Subject = $title; //邮件标题
        $mail->Body = $content; //邮件内容
        //$mail->AltBody = "This is the body in plain text for non-HTML mail clients"; //附加信息，可以省略
 
        if(!$mail->Send())
        {
            return $mail->ErrorInfo;
        }
        return "ok";
    }
	
	
	
	public function updatepwd(){
		if($_SERVER['REQUEST_METHOD'] == 'GET'){
			$this->assign('mail', $_GET['mail']);
			$this->display();	
		}else{
			if(!empty($_POST['mail']) && !empty($_POST['pwd'])){
				$client = M('client');
				$pwd = md5(trim($_POST['pwd'])); //md5()
				$ret = $client->where("mail='".$_POST['mail']."'")->find();
				
				$is_ok = $client->where("mail='".$_POST['mail']."'")->setField('pwd',$pwd);
				
				if($is_ok===false){
					$this->ajaxReturn('修改密码错误，请联系管理员！', 'error',0);
				}else{					
					$_SESSION['user'] = $ret["mail"];
					$_SESSION['id'] = $ret['id'];
					$this->ajaxReturn("修改密码成功！3秒自动跳转域名管理页面", 'success',1);
				}
			}
		}
	}
	
	public function userUpdatepwd(){
		if(!empty($_SESSION['user'])){
			if($_SERVER['REQUEST_METHOD'] == 'GET'){
				
				$this->assign('mail', $_GET['mail']);
				$this->display();
			}else{
				if(!empty($_POST['mail']) && !empty($_POST['oldpwd']) && !empty($_POST['newpwd'])){
					
					$cnd["mail"] = trim($_POST['mail']);
					$cnd["pwd"] = md5(trim($_POST['oldpwd'])); //md5()
					
					$list = M("client");
					$rslt = $list->where($cnd)->find();
					
					if(empty($rslt)){
						$this->ajaxReturn('对不起，您输入的旧密码不正确。','error',0);
					}
				
					$pwd = md5(trim($_POST['newpwd'])); //md5()
					$is_ok = $list->where("mail='".$_POST['mail']."'")->setField('pwd',$pwd);
					
					if($is_ok===false){
						$this->ajaxReturn('修改密码错误，请联系管理员！', 'error',0);
					}else{					
						$_SESSION['user'] = $rslt["mail"];
						$_SESSION['id'] = $rslt['id'];
						$this->ajaxReturn("修改密码成功！3秒自动跳转用户信息", 'success',1);
					}
					
				}
			}
		}else{
			header("Location: ".__APP__."/Index/login");
		}
	}
	
	
	
}
