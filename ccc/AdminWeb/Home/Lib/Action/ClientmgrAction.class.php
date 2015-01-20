<?php
// 本类由系统自动生成，仅供测试用途
class ClientmgrAction extends BaseAction {

	public function index(){
		$list = M('client',Null,'DB_NEWS'); 
		$map["type"] = 0;
		//非管理员
		if($_SESSION['level'] != 1){
			$map['internal'] = 0;
		}
		if(!empty($_GET['mail'])){
			$map["mail"] = array('like','%'.$_GET['mail'].'%');
			$this->assign('mail',$_GET['mail']);
		}
		import('ORG.Util.Page');// 导入分页类
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$clist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录";
		}
		$this->assign('clist',$clist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	public function savePwd(){
		if(!empty($_POST['id']) && !empty($_POST['pwd'])){
			$client	= M('client',Null,'DB_NEWS'); 
			$data['pwd'] = md5($_POST['pwd']);			
			$is_ok = $client->where('id='.$_POST['id'])->save($data);
			if($is_ok === false){
				$this->ajaxReturn('修改用户密码失败，请联系管理员','error',0);
			}
			$this->ajaxReturn('修改用户密码成功','success',1);
		}
	}
	public function updateInternalClient(){
		if(!empty($_POST['id'])){					
			$client	= M('client',Null,'DB_NEWS'); 			
			$is_ok = $client->where('id='.$_POST['id'])->setField('internal',$_POST['internal']);
			if($is_ok === false){
				if($_POST['internal'] == 1){
					$this->ajaxReturn('设置内部账户失败，请联系管理员','error',0);
				}else{
					$this->ajaxReturn('设置外部账户失败，请联系管理员','error',0);
				}
			}
			if($_POST['internal'] == 1){
				$this->ajaxReturn("设置内部账户成功",'success',1);
			}else{
				$this->ajaxReturn("设置外部账户成功",'success',1);
			}
		}
	}
	public function updateClientStatus(){
		if(!empty($_POST['id'])){
			$client	= M('client',Null,'DB_NEWS'); 
			$is_ok = $client->where("id=".$_POST['id'])->setField('status',$_POST['status']);	
			if($is_ok === false){
				if($_POST['status'] == 1){
					$this->ajaxReturn('冻结账户失败，请联系管理员','error',0);
				}else{
					$this->ajaxReturn('解冻账户失败，请联系管理员','error',0);
				}
			}
			if($_POST['status'] == 1){
				$this->ajaxReturn("冻结账户成功",'success',1);
			}else{
				$this->ajaxReturn("解冻账户成功",'success',1);
			}
		}
	}
	public function resetPwd(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$client = M('client',Null,'DB_NEWS'); 
			$user = $client->where('id='.$_GET['id'])->find();
			$this->assign('user',$user);
			$this->display();
		}else{
			if(!empty($_POST['id']) && !empty($_POST['pwd'])){
				$client	= M('client',Null,'DB_NEWS'); 
				$is_ok = $client->where("id=".$_POST['id'])->setField('pwd',md5($_POST['pwd']));
				if($is_ok === false){
					$this->ajaxReturn('重置密码失败，请联系管理员','error',0);
				}
				$this->ajaxReturn("重置密码成功",'success',1);
			}
		}
	}
	public function addClient(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(!empty($_POST['mail']) && !empty($_POST['pwd']) && !empty($_POST['type'])){
			
				$data = file_get_contents(C('API_URL')."/user.php?opt=insert&mail=".$_POST['mail']."&pwd=".$_POST['pwd']."&type=".$_POST['type']);				
				$result = json_decode($data,true);	
				if($result['ret']!=0){
					$this->ajaxReturn($result['error'],'error',0);
				}				
				$this->ajaxReturn('添加用户成功','success',1);
			}
		}
	}
	public function setInfo(){		
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			if(!empty($_GET['id'])){
				$client = M('client',Null,'DB_NEWS'); 
				$user = $client->where('id='.$_GET['id'])->find();
				$this->assign('user',$user);
				$this->display();
			}
		}else{
			$data['org_name'] = $_POST['org_name'];
			$data['org_num'] = $_POST['org_num'];
			$data['link_phone'] = $_POST['link_phone'];
			$data['link_man'] = $_POST['link_man'];
			$data['link_weixin'] = $_POST['link_weixin'];
			$data['status'] = $_POST['status'];
			
			$client = M('client',Null,'DB_NEWS'); 
			$is_ok = $client->where('id='.$_POST['id'])->save($data);
			if($is_ok === false){
				$this->ajaxReturn('修改客户信息失败，请联系管理员','error',0);
			}
			$this->ajaxReturn('修改客户信息成功','success',1);
			
		}
	}
	public function vip(){
		$list = M('client',Null,'DB_NEWS'); 
		$map["type"] = 1;	
		//非管理员
		if($_SESSION['level'] != 1){
			$map['internal'] = 0;
		}
		if(!empty($_GET['mail'])){
			$map["mail"] = array('like','%'.$_GET['mail'].'%');
			$this->assign('mail',$_GET['mail']);
		}
		import('ORG.Util.Page');// 导入分页类
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$clist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录";
		}
		$this->assign('clist',$clist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function mgrClient(){
		$this->assign('mail',$_GET['mail']);
		$this->assign('pwd',$_GET['pwd']);
		$this->display();	
	}
	
	public function readInfo(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('client',Null,'DB_NEWS'); 
			$zone = M('zone',Null,'DB_NEWS'); 
			$data = $list->where('id='.$_POST['id'])->find();
			$zlist = $zone->where('client_id='.$_POST['id'])->select();
			$str = " ";
			if(!empty($zlist)){
				foreach($zlist as $z){
					$str .= $z['domain'] . " ， " ;
				}
				$str = substr($str,0,-1);
			}
			$data['domain'] = $str;
			$this->ajaxReturn($data,'',1);
		}
	}
	
	public function delClient(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			if(!empty($_POST['id'])){
				$client = M('client',Null,'DB_NEWS');
				$user = $client->where('id='.$_POST['id'])->find();
				//删除接口
				$data = file_get_contents(C('API_URL')."/user.php?opt=delete&mail=".$user['mail']);
				$result = json_decode($data,true);
				if($result['ret']==0){
					$this->ajaxReturn('用户已经删除！','success',1);
				}else{
					$this->ajaxReturn('用户删除失败！','error',0);
				}
			}			
		}
	}
	
	
}
