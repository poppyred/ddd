<?php
// 本类由系统自动生成，仅供测试用途
class RunmgrAction extends Action {

	public function index(){
		$list = M('view',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$vlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->select();
		//print_r($list->getlastsql());exit;
		$show       = $Page->show();// 分页显示输出
		$this->assign('vlist', $vlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function editView(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$cnd["id"] = trim($_GET['id']);
			$list = M('view',Null,'DB_NEWS'); 
			$vo = $list->where($cnd)->find();
			$this->assign('view', $vo);
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
	
	public function addView(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(isset($_POST['name']) && isset($_POST['level']) && isset($_POST['mask'])){
				$data["name"] = trim($_POST['name']);
				$data["level"] = trim($_POST['level']);
				$data["up_time"] = date('y-m-d h:i:s');
				$list = M('view',Null,'DB_NEWS'); 
				$is_ok = $list->data($data)->add();
				//添加成功
				if($is_ok){
					$view_mask = M('view_mask',Null,'DB_NEWS'); 
					$str = trim($_POST['mask']);
					$arr = $explore(',', $str);
					for($i=0;$i<count($arr);$i++){
						$m = $explore('-', $arr[$i]);
						$mask['mask_start'] = ip2long($m[0]);
						$mask['mask_end'] = ip2long($m[1]);
						$mask['view_id'] = $is_ok;
						$view_mask->data($mask)->add();
					}
					$this->ajaxReturn(1,'默认路线添加成功！',1); //data,info,status
				}else{
					$this->ajaxReturn(0,'添加失败，联系管理员',0);
				}
			}else{
				$this->ajaxReturn(0,'添加失败，缺少参数',0);
			}
		}
	}
	
	public function route(){
		$list = M('route',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$rlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->select();
		//print_r($list->getlastsql());exit;
		$show       = $Page->show();// 分页显示输出
		$this->assign('rlist', $rlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function addRoute(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(isset($_POST['name']) && isset($_POST['zone']) && isset($_POST['mask'])){
				$zone = M('zone',Null,'DB_NEWS'); 
				$cnd["domain"] = trim($_POST['zone']);
				$ret = $zone->where($cnd)->find();
				if(empty($ret)){
					$this->ajaxReturn(0,'添加失败，没有找到对应的域名！',0);
				}else{
					$data["name"] = trim($_POST['name']);
					$data["zone_id"] = $ret['id'];
					$data["up_time"] = date('y-m-d h:i:s');
					$list = M('route',Null,'DB_NEWS'); 
					$is_ok = $list->data($data)->add();
					//添加成功
					if($is_ok){
						$view_mask = M('route_mask',Null,'DB_NEWS'); 
						$str = trim($_POST['mask']);
						$arr = $explore(',', $str);
						for($i=0;$i<count($arr);$i++){
							$m = $explore('-', $arr[$i]);
							$mask['mask_start'] = ip2long($m[0]);
							$mask['mask_end'] = ip2long($m[1]);
							$mask['route_id'] = $is_ok;
							$view_mask->data($mask)->add();
						}
						$this->ajaxReturn(1,'自定义路线添加成功！',1); //data,info,status
					}else{
						$this->ajaxReturn(0,'添加失败，联系管理员',0);
					}
				}
			}else{
				$this->ajaxReturn(0,'添加失败，缺少参数',0);
			}
		}
	}
	
	public function normalns(){
		$list = M('nameserver',Null,'DB_NEWS'); 
		$rlist = $list->where('level=0')->select();
		$view = M('view',Null,'DB_NEWS'); 
		for($i=0;$i<count($rlist);$i++){
			$cnd["id"] = $rlist[$i]["view"];
			$ret = $view->where($cnd)->find();
			$rlist[$i]["view_name"] = $ret["name"];
		}
		$this->assign('rlist', $rlist);// 赋值数据集
		$this->display();
	}
	
	public function vipns(){
		$list = M('nameserver',Null,'DB_NEWS'); 
		$rlist = $list->where('level=1')->select();
		$view = M('view',Null,'DB_NEWS'); 
		for($i=0;$i<count($rlist);$i++){
			$cnd["id"] = $rlist[$i]["view"];
			$ret = $view->where($cnd)->find();
			$rlist[$i]["view_name"] = $ret["name"];
		}
		$this->assign('rlist', $rlist);// 赋值数据集
		$this->display();
	}
	
	public function addns(){
		$list = M('view',Null,'DB_NEWS'); 
		$rlist = $list->select();
		$this->assign('vlist', $rlist);// 赋值数据集
		$this->display();
	}
	
	public function editns(){
		if(isset($_GET['id'])){
			$list = M('view',Null,'DB_NEWS'); 
			$rlist = $list->select();
			$this->assign('vlist', $rlist);// 赋值数据集
			
			$name = M('nameserver',Null,'DB_NEWS'); 
			$cnd["id"] = $_GET['id'];
			$ret = $name->where($cnd)->find();
			$this->assign('model', $ret);
			$this->display();
		}
	}
	
}
