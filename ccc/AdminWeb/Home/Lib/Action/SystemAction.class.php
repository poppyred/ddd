<?php
// 本类由系统自动生成，仅供测试用途
class SystemAction extends BaseAction {

	public function index(){
		$list = M('server_list');
		$type = M('type');
		$subtype = M('subtype');
		$tlist = $type->select();
		$stlist = $subtype->select();
		$this->assign('stlist',$stlist);
		$this->assign('tlist',$tlist);
		if(!empty($_GET['t'])){
			$map['type'] = $_GET['t'];
			$this->assign('type', $_GET['t']);
		}
		if(!empty($_GET['st'])){			
			$map['subtype'] = $_GET['st'];
			$this->assign('subtype', $_GET['st']);
		}
		$vo = $list->where($map)->select();
		$this->assign('slist', $vo);
		$this->display();
	}
	public function editServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$cnd["id"] = trim($_GET['id']);
			$list = M('server_list');
			$type = M('type');
			$subtype = M('subtype');
			$vo = $list->where($cnd)->find();
			//print_r($list->getLastSQL());exit;
			$this->assign('server', $vo);
			//print_r($vo);exit;
			$tlist = $type->select();
			$stlist = $subtype->select();
			$this->assign('tlist',$tlist);
			$this->assign('ctlist',$stlist);
			$this->display();
		}else{
			if(isset($_POST['sid']) && isset($_POST['ip'])){
				$list = M("server_list");
				$entity = $list->where("sid='".$_POST['sid']."'")->find();
				if(!empty($entity)){
					$this->ajaxReturn(0,'输入的服务器编码重复',0);	
				}
				$data["sid"] = trim($_POST['sid']);
				$data["ip"] = trim($_POST['ip']);
				$data["desc"] = trim($_POST['desc']);
				$data["type"] = trim($_POST['type']);
				$data["status"] = trim($_POST['status']);
				$data["subtype"] = trim($_POST['subtype']);
				$is_ok = $list->where('id='.$_POST['id'])->save($data);
				//添加成功
				if($is_ok === false){
					$this->ajaxReturn(0,'修改失败，联系管理员',0);
				}else{
					$this->ajaxReturn(1,'服务器修改成功！',1); //data,info,status
				}
			}
		}
	}
	
	public function deleteServer(){
		if(!empty($_POST['id'])){
			$list = M('server_list');
			$is_ok = $list->where('id='.$_POST['id'])->delete();
			if($is_ok === false){
				$this->ajaxReturn('删除服务器失败，请联系管理员','error',0);	
			}
			$this->ajaxReturn(1,'success',1);
		}
	}
	public function addServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$list = M('server_list');
			$type = M('type');
			$subtype = M('subtype');
			$tlist = $type->select();
			$stlist = $subtype->select();
			$this->assign('tlist',$tlist);
			$this->assign('ctlist',$stlist);
			$this->display();
		}else{
			if(isset($_POST['sid']) && isset($_POST['ip'])){
				$list = M("server_list");
				$entity = $list->where("sid='".$_POST['sid']."'")->find();
				if(!empty($entity)){
					$this->ajaxReturn(0,'输入的服务器编码重复',0);	
				}
				
				$data["sid"] = trim($_POST['sid']);
				$data["ip"] = trim($_POST['ip']);
				$data["desc"] = trim($_POST['desc']);
				$data["type"] = trim($_POST['type']);
				$data["status"] = trim($_POST['status']);
				$data["subtype"] = trim($_POST['subtype']);
				
				$is_ok = $list->add($data);
				//添加成功
				if($is_ok){
					$this->ajaxReturn(1,'服务器添加成功！',1); //data,info,status
				}else{
					$this->ajaxReturn(0,'添加失败，联系管理员',0);
				}
			}
		}
	}
	
	public function addUser(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(isset($_POST['user']) && isset($_POST['pwd'])){				
				$list = M("admin");
				$entity = $list->where("user='".$_POST['user']."'")->find();
				if(!empty($entity)){
					$this->ajaxReturn(0,'输入的用户名已存在',0);
				}
				
				$data["user"] = trim($_POST['user']);
				$data["pwd"] = trim($_POST['pwd']);
				$data["level"] = trim($_POST['level']);
				$data["desc"] = trim($_POST['desc']);
				$is_ok = $list->add($data);
				//添加成功
				if($is_ok){
					$this->ajaxReturn(1,'用户添加成功！',1); //data,info,status
				}else{
					$this->ajaxReturn(0,'添加失败，联系管理员',0);
				}
			}
		}
	}
	
	public function editUser(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			if(!empty($_GET['id'])){
				$admin = M('admin');
				$tem = $admin->where('id='.$_GET['id'])->find();
				$this->assign('admin',$tem);
				$this->display();	
			}
		}else{
			if(isset($_POST['user']) && isset($_POST['pwd'])){
				$list = M("admin");
				$entity = $list->where("user='".$_POST['user']."'")->find();
				if(!empty($entity)){
					$this->ajaxReturn(0,'输入的用户名已存在',0);
				}
				$data["user"] = trim($_POST['user']);
				$data["pwd"] = trim($_POST['pwd']);
				$data["level"] = trim($_POST['level']);
				$data["desc"] = trim($_POST['desc']);
				$is_ok = $list->where('id='.$_POST['id'])->save($data);
				//添加成功
				if($is_ok===false){					
					$this->ajaxReturn(0,'修改失败，联系管理员',0);
				}else{
					$this->ajaxReturn(1,'用户修改成功！',1); //data,info,status
				}
			}
		}
	}
	
	public function stopServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('server_list');
			$list->where('id='.$_POST['id'])->setField('status','false');
			$this->ajaxReturn(1,'服务器已经停用！',1);
		}
	}
	public function startServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('server_list');
			$list->where('id='.$_POST['id'])->setField('status','true');
			$this->ajaxReturn(1,'服务器已经启用！',1);
		}
	}
	
	public function user(){
		$list = M('admin');
		$ulist = $list->select();
		$this->assign('ulist',$ulist);
		$this->display();
	}
	
	public function delUser(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('admin');
			$list->where('id='.$_POST['id'])->delete();
			$this->ajaxReturn(1,'用户已经删除！',1);
		}
	}
	
	public function task(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){	
			$list = M('task_list');					
			//$map["type"] = 0;
			import('ORG.Util.Page');// 导入分页类
			if(!empty($_GET['t'])){
				$map['type'] = $_GET['t'];
				$this->assign('type',$_GET['t']);
			}
			if(!empty($_GET['o'])){
				$map['opt'] = $_GET['o'];
				$this->assign('opt',$_GET['o']);
			}
			$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件 ->where($map)
			$Page = new Page($count, 20);// 实例化分页类 传入总记录数
			$nowPage = isset($_GET['p'])?$_GET['p']:1;
			$tlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();//where($map)->select();//$list->order('id desc')->select();
			$show = $Page->show();// 分页显示输出
			if(empty($show)){
				$show = " 0 条记录"	;
			}
			$this->assign('page',$show);// 赋值分页输出
			$this->assign('tlist',$tlist);
			$this->display();
		}else{
			$list = M('task_feedback');
			$rlist = $list->where('taskid='.$_POST['id'])->select();
			$this->ajaxReturn($rlist,'success',1);
		}
	}
	public function reTask(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('task_list');
			$cnd["taskid"] = $_POST['tid'];
			$t = $list->where($cnd)->find();
			$data["retry"] = $t["retry"] - 1;
			$data["error"] = "";
			$data["result"] = "";
			$data["status"] = "init";
			$is_ok = $list->where($cnd)->save($data);
			if($is_ok){
				$this->ajaxReturn(1,'任务下发成功！',1);
			}else{
				$this->ajaxReturn(0,'任务下发失败！',0);
			}
		}
	}
}
