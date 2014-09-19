<?php
// 本类由系统自动生成，仅供测试用途
class SystemAction extends Action {

	public function index(){
		$list = M('server_list');
		$tlist = $list->Distinct(true)->field('type')->select();
		$this->assign('tlist',$tlist);
		
		if(!empty($_GET['t'])){
			$vo = $list->where("type='".$_GET['t']."'")->select();
			$stlist = $list->where("type='".$_GET['t']."'")->Distinct(true)->field('subtype')->select();
			$this->assign('ctlist',$stlist);
			$this->assign('type', $_GET['t']);
		}else{
			$vo = $list->select();
			$stlist = $list->Distinct(true)->field('subtype')->select();
			$this->assign('ctlist',$stlist);
			$this->assign('type', "");
		}
		
		$this->assign('slist', $vo);
		$this->display();
	}
	public function editServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$cnd["id"] = trim($_GET['id']);
			$list = M('server_list');
			$vo = $list->where($cnd)->find();
			//print_r($list->getLastSQL());exit;
			$this->assign('server', $vo);
			//print_r($vo);exit;
			$tlist = $list->Distinct(true)->field('type')->select();
			$stlist = $list->Distinct(true)->field('subtype')->select();
			$this->assign('tlist',$tlist);
			$this->assign('ctlist',$stlist);
			$this->display();
		}else{
			if(isset($_POST['sid']) && isset($_POST['ip'])){
				$data["sid"] = trim($_POST['sid']);
				$data["ip"] = trim($_POST['ip']);
				$data["desc"] = trim($_POST['desc']);
				$data["type"] = trim($_POST['type']);
				$data["status"] = trim($_POST['status']);
				$data["subtype"] = trim($_POST['subtype']);
				$list = M("server_list");
				$is_ok = $list->where('id='.$_POST['id'])->save($data);
				//添加成功
				if($is_ok){
					$this->ajaxReturn(1,'服务器修改成功！',1); //data,info,status
				}else{
					$this->ajaxReturn(0,'修改失败，联系管理员',0);
				}
			}
		}
	}
	public function addServer(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$list = M('server_list');
			$tlist = $list->Distinct(true)->field('type')->select();
			$stlist = $list->Distinct(true)->field('subtype')->select();
			$this->assign('tlist',$tlist);
			$this->assign('ctlist',$stlist);
			$this->display();
		}else{
			if(isset($_POST['sid']) && isset($_POST['ip'])){
				$data["sid"] = trim($_POST['sid']);
				$data["ip"] = trim($_POST['ip']);
				$data["desc"] = trim($_POST['desc']);
				$data["type"] = trim($_POST['type']);
				$data["status"] = trim($_POST['status']);
				$data["subtype"] = trim($_POST['subtype']);
				$list = M("server_list");
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
				$data["user"] = trim($_POST['user']);
				$data["pwd"] = trim($_POST['pwd']);
				$data["desc"] = trim($_POST['desc']);
				$list = M("admin");
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
			$tlist = $list->order('id desc')->select();
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
