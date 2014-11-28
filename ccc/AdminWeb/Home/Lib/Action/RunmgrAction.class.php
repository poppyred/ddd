<?php
// 本类由系统自动生成，仅供测试用途
class RunmgrAction extends BaseAction {

	public function index(){
		$list = M('view',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		if(!empty($_GET['view'])){
			$map['name'] = array('like','%'.$_GET['view'].'%');
			$this->assign('view', $_GET['view']);
		}
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$vlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录" ;
		}
		$this->assign('vlist', $vlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	public function deleteViewMask(){
		if(!empty($_POST['id'])){
			$view_mask = M('view_mask',Null,'DB_NEWS');
			$is_ok = $view_mask->where('id='.$_POST['id'])->delete();
			if($is_ok === false){
				$this->ajaxReturn('删除线路掩码失败','error',0);
			}else{
				$vm = $view_mask->where('id='.$_POST["id"])->find();
				
				$val = array("mask"=>$vm['network'],"vid"=>$_POST["id"]);
				$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
				$param = array("type"=>"mask", "opt"=>"del", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,$rslt["content"],0);	
				}
				
				$this->ajaxReturn(1,'success',1);
			}
		}
	}
	public function addMask(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){			
			$view = M('view',Null,'DB_NEWS');			
			$vlist = $view->select();
			$this->assign('vlist',$vlist);
			$this->display();
		}else{
			if(!empty($_POST['viewid']) && !empty($_POST['mask'])){	
				$view_mask = M('view_mask',Null,'DB_NEWS');			
				$str = trim($_POST['mask']);
				$arr = explode(',', $str);
				for($i=0;$i<count($arr);$i++){
					if(!empty($arr[$i])){						
						$mask['network'] = $arr[$i];
						$mask['viewid'] = $_POST['viewid'];
						$is_ok = $view_mask->data($mask)->add();
						if($is_ok === false){
							$this->ajaxReturn('error','添加掩码失败，请联系管理员。',0);
						}
						$val = array("mask"=>$arr[$i],"vid"=>$_POST["viewid"]);
						$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
						$param = array("type"=>"mask", "opt"=>"add", "data"=>$val,"user"=>$user);
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
						$rslt = json_decode($ret["content"],true);
						if($rslt["ret"] != 0){
							$this->ajaxReturn(0,$rslt["content"],0);	
						}
					}
				}
				$this->ajaxReturn(1,'success',1);	
			}
		}
	}
	public function mask(){
		if(!empty($_GET['id'])){
			$view = M('view',Null,'DB_NEWS');
			$view_mask = M('view_mask',Null,'DB_NEWS');
			import('ORG.Util.Page');// 导入分页类
			$map['viewid'] = $_GET['id'];
			if(!empty($_GET['network'])){
				$map['network'] = array('like','%'.$_GET['network'].'%');
				$this->assign('network', $_GET['network']);
			}
			$count = $view_mask->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
			$Page = new Page($count, 50);// 实例化分页类 传入总记录数
			$nowPage = isset($_GET['p'])?$_GET['p']:1;
			
			$vo = $view->where('id='.$_GET['id'])->find();
			$vlist = $view_mask->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
			
			$show = $Page->show();// 分页显示输出
			if(empty($show)){
				$show = " 0 条记录" ;
			}
			$this->assign('page',$show);// 赋值分页输出
			$this->assign('vlist',$vlist);
			$this->assign('view',$vo['name']);
			$this->assign('id',$_GET['id']);
			$this->display();	
		}
	}
	public function saveViewMask(){
		if(!empty($_POST['id']) && !empty($_POST['network'])){
			$view_mask = M('view_mask',Null,'DB_NEWS');
			$data['network'] = $_POST['network'];
			$is_ok = $view_mask->where('id='.$_POST['id'])->save($data);
			if($is_ok === false){
				$this->ajaxReturn('修改掩码失败','error',0);
			}
			$vm = $view_mask->where('viewid='.$_POST['vid'])->select();
			$this->ajaxReturn($vm,'success',1);
		}
	}
	
	public function editView(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$cnd["id"] = trim($_GET['id']);
			$list = M('view',Null,'DB_NEWS'); 
			$vo = $list->where($cnd)->find();			
			$view_mask = M('view_mask',Null,'DB_NEWS');
			$vm = $view_mask->where('viewid='.$_GET['id'])->select();
			/*$str;
			foreach($vm as $item){
				$str .= $item['network'] . ',';
			}
			$str = substr($str,0,-1);*/
			$this->assign('view', $vo);
			$this->assign('vmlist', $vm);
//			print_r($vm);exit;
			$this->display();
		}else{
			if(isset($_POST['id']) && isset($_POST['name']) && isset($_POST['level'])){//&& isset($_POST['mask'])
				$data['name'] = $_POST['name'];
				$data['level'] = $_POST['level'];
				$data['desc'] = $_POST['desc'];
				$view = M('view',Null,'DB_NEWS'); 
				$view_mask = M('view_mask',Null,'DB_NEWS'); 
				$is_ok = $view->where('id='.$_POST['id'])->save($data);
				
				if($is_ok===false){
					$this->ajaxReturn('修改失败，请联系管理员','error',0);	
				}
				$val = array("vid"=>(int)$_POST['id'],"vname"=>rawurldecode($_POST["name"]),"comment"=>rawurldecode($_POST["desc"]),"ttl"=>"300");
				$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
				$param = array("type"=>"view", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,$rslt["content"],0);	
				}
				
				//删除view_mask
				/*
				$vmlist = $view_mask->where('viewid='.$_POST['id'])->select();
				
				foreach($vmlist as $vm){
					$val = array("mask"=>$vm['network'],"vid"=>$vm["viewid"]);
					$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
					$param = array("type"=>"mask", "opt"=>"del", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					if($rslt["ret"] != 0){
						$this->ajaxReturn(0,$rslt["content"],0);	
					}
				}
				
				
				$view_mask->where('viewid='.$_POST['id'])->delete();
				
				
				$str = trim($_POST['mask']);
				$arr = explode(',', $str);
				for($i=0;$i<count($arr);$i++){
					if(!empty($arr[$i])){
						//$m = explode('-', $arr[$i]);
						//$mask['mask_start'] = ip2long($m[0]);
						//$mask['mask_end'] = ip2long($m[1]);
						$mask['network'] = $arr[$i];
						$mask['viewid'] = $_POST['id'];
						$view_mask->data($mask)->add();
						
						$val = array("mask"=>$mask['network'],"vid"=>$mask["viewid"]);
						$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
						$param = array("type"=>"mask", "opt"=>"add", "data"=>$val,"user"=>$user);
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
						$rslt = json_decode($ret["content"],true);
						if($rslt["ret"] != 0){
							$this->ajaxReturn(0,$rslt["content"],0);	
						}
					}
				}
				*/
				$this->ajaxReturn(1,'默认路线修改成功！',1);
			}
		}
	}
	
	public function deleteView(){
		if(!empty($_POST['id'])){
			$view = M('view',Null,'DB_NEWS');
			$view_mask = M('view_mask',Null,'DB_NEWS');
			$vmlist = $view_mask->where('viewid='.$_POST['id'])->select();
			//删除mask
			foreach($vmlist as $vm){
				$val = array("mask"=>$vm['network'],"vid"=>$vm["viewid"]);
				$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
				$param = array("type"=>"mask", "opt"=>"del", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,$rslt["content"],0);	
				}
			}
			
			$is_ok = $view_mask->where('viewid='.$_POST['id'])->delete();
			if($is_ok===false){
				$this->ajaxReturn('删除路线失败，请联系管理员','error',0);
			}
			//
			$val = array("vid"=>$_POST['id']);
			$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
			$param = array("type"=>"view", "opt"=>"del", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] != 0){
				$this->ajaxReturn(0,$rslt["content"],0);	
			}
			
			$is_ok2 = $view->where('id='.$_POST['id'])->delete();
			if($is_ok2===false){
				$this->ajaxReturn('删除路线失败，请联系管理员','error',0);
			}
			
			$this->ajaxReturn(1,'success',1);
		}
	}
	public function addView(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(isset($_POST['name']) && isset($_POST['level']) && isset($_POST['mask'])){
				
				
				$data["name"] = trim($_POST['name']);
				$data["level"] = trim($_POST['level']);
				$data["desc"] = trim($_POST['desc']);
				$data["up_time"] = date('Y-m-d H:i:s');
				$list = M('view',Null,'DB_NEWS');
				$is_ok = $list->data($data)->add();
				
				$val = array("vid"=>(int)$is_ok,"vname"=>rawurldecode($data["name"]),"comment"=>rawurldecode($data["desc"]),"ttl"=>"300");
				$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
				$param = array("type"=>"view", "opt"=>"add", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,$rslt["content"],0);	
				}
				
				//添加成功
				if($is_ok){
					$view_mask = M('view_mask',Null,'DB_NEWS'); 
					$str = trim($_POST['mask']);
					$arr = explode(',', $str);
					for($i=0;$i<count($arr);$i++){
						if(!empty($arr[$i])){
							//$m = explode('-', $arr[$i]);
							//$mask['mask_start'] = ip2long($m[0]);
							//$mask['mask_end'] = ip2long($m[1]);							
							$mask['network'] = $arr[$i];
							$mask['viewid'] = $is_ok;
							$view_mask->data($mask)->add();
							
							$val = array("mask"=>$vm['network'],"vid"=>$mask["viewid"]);
							$user = array("cid"=>$_SESSION['id'], "level"=>2, "info"=>"");
							$param = array("type"=>"mask", "opt"=>"add", "data"=>$val,"user"=>$user);
							$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
							$rslt = json_decode($ret["content"],true);
							if($rslt["ret"] != 0){
								$this->ajaxReturn(0,$rslt["content"],0);	
							}
							
						}
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
		if(!empty($_GET['route'])){
			$map['name'] = array('like','%'.$_GET['route'].'%');
			$this->assign('route',$_GET['route']);
		}
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$rlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录" ;
		}
		foreach($rlist as $key => $val){
			$zone = M('zone',Null,'DB_NEWS');
			$tem = $zone->where('id='.$val['zone_id'])->find();
			$rlist[$key]['zone'] = $tem['domain'];
		}
		$this->assign('rlist', $rlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	public function editRoute(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){			
			$route = M('route',Null,'DB_NEWS'); 
			$zone = M('zone',Null,'DB_NEWS'); 
			$list = $zone->select();
			$result = $route->where('id='.$_GET['id'])->find();
			$route_mask = M('route_mask',Null,'DB_NEWS');
			$rm = $route_mask->where('route_id='.$_GET['id'])->select();
			$str;
			foreach($rm as $item){
				$str .= long2ip($item['mask_start']) . '-' .long2ip($item['mask_end']) . ',';
			}
			$str = substr($str,0,-1);
			$this->assign('str', $str);
			$this->assign('result',$result);
			$this->assign('zlist',$list);
			$this->display();
		}else{
			if(isset($_POST['name']) && isset($_POST['zone']) && isset($_POST['mask']) && isset($_POST['id'])){
				$data['name'] = $_POST['name'];
				$data["zone_id"] = $_POST['zone'];
				$route = M('route',Null,'DB_NEWS'); 
				$is_ok = $route->where('id='.$_POST['id'])->save($data);
				//删除view_mask
				$route_mask = M('route_mask',Null,'DB_NEWS'); 
				$route_mask->where('route_id='.$_POST['id'])->delete();				
				$str = trim($_POST['mask']);
				$arr = explode(',', $str);
				for($i=0;$i<count($arr);$i++){
					if(!empty($arr[$i])){
						$m = explode('-', $arr[$i]);
						$mask['mask_start'] = ip2long($m[0]);
						$mask['mask_end'] = ip2long($m[1]);
						$mask['route_id'] = $_POST['id'];
						$route_mask->data($mask)->add();
					}
				}
				$this->ajaxReturn(1,'默认路线修改成功！',1);
				
			}
		}
	}
	public function deleteRoute(){
		if(!empty($_POST['id'])){
			$route = M('route',Null,'DB_NEWS'); 	
			$route_mask = M('route_mask',Null,'DB_NEWS'); 	
			
			$is_ok = $route->where('id='.$_POST['id'])->delete();
			$is_ok2 = $route_mask->where('route_id='.$_POST['id'])->delete();
			if($is_ok === false || $is_ok2 === false){
				$this->ajaxReturn('删除自定义路线失败。','error',0);
			}		
			$this->ajaxReturn(1,'success',1);
		}
	}
	public function addRoute(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$zone = M('zone',Null,'DB_NEWS'); 
			$list = $zone->select();
			$this->assign('zlist',$list);
			$this->display();
		}else{
			if(isset($_POST['name']) && isset($_POST['zone']) && isset($_POST['mask'])){
				//$zone = M('zone',Null,'DB_NEWS'); 
				//$cnd["domain"] = trim($_POST['zone']);
				//$ret = $zone->where($cnd)->find();
				//if(empty($ret)){
				//	$this->ajaxReturn(0,'添加失败，没有找到对应的域名！',0);
				//}else{
					$data["name"] = trim($_POST['name']);
					$data["zone_id"] = $_POST['zone'];
					$data["up_time"] = date('y-m-d h:i:s');
					$list = M('route',Null,'DB_NEWS'); 
					$is_ok = $list->data($data)->add();
					//添加成功
					if($is_ok){
						$view_mask = M('route_mask',Null,'DB_NEWS'); 
						$str = trim($_POST['mask']);
						$arr = explode(',', $str);
						for($i=0;$i<count($arr);$i++){
							if(!empty($arr[$i])){
								$m = explode('-', $arr[$i]);
								$mask['mask_start'] = ip2long($m[0]);
								$mask['mask_end'] = ip2long($m[1]);
								$mask['route_id'] = $is_ok;
								$view_mask->data($mask)->add();
							}
						}
						$this->ajaxReturn(1,'自定义路线添加成功！',1); //data,info,status
					}else{
						$this->ajaxReturn(0,'添加失败，联系管理员',0);
					}
				//}
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
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$list = M('view',Null,'DB_NEWS'); 
			$rlist = $list->select();
			$this->assign('vlist', $rlist);// 赋值数据集
			$this->display();
		}else{
			if(isset($_POST['val']) && isset($_POST['level']) && isset($_POST['ttl']) && isset($_POST['view'])){
				$nameserver = M('nameserver',Null,'DB_NEWS'); 
				$data['level'] = $_POST['level'];
				$data['val'] = $_POST['val'];
				$data['view'] = $_POST['view'];
				$data['ttl'] = $_POST['ttl'];
				$is_ok = $nameserver->data($data)->add();
				if($is_ok === false){
					$this->ajaxReturn(0,'添加默认NS记录失败',0);
				}
				$this->ajaxReturn(1,'添加默认NS记录成功',1);
			}
		}		
	}
	
	public function deleteNS(){
		if(isset($_POST['id'])){
			$nameserver = M('nameserver',Null,'DB_NEWS');
			$is_ok = $nameserver->where('id='.$_POST['id'])->delete();
			if($is_ok === false){				
				$this->ajaxReturn('删除默认NS记录失败','error',0);
			}			
			$this->ajaxReturn(1,'success',1);
		}
	}
	
	public function editns(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
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
		}else{
			if(isset($_POST['val']) && isset($_POST['level']) && isset($_POST['ttl']) && isset($_POST['view']) && isset($_POST['id'])){
				$data['level'] = $_POST['level'];
				$data['val'] = $_POST['val'];
				$data['view'] = $_POST['view'];
				$data['ttl'] = $_POST['ttl'];
				$name = M('nameserver',Null,'DB_NEWS'); 
				$ret = $name->where('id='.$_POST['id'])->save($data);
				if($ret === false){
					$this->ajaxReturn(0,'修改默认NS记录失败',0);
				}
					$this->ajaxReturn(1,'修改默认NS记录成功',1);
			}
		}
	}
	
}
