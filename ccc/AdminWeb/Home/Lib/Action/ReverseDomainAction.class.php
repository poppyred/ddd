<?php
// 本类由系统自动生成，仅供测试用途
class ReverseDomainAction extends BaseAction {
	public function index(){
//		$this->assign('rlist',$this->returnList());	

		import('ORG.Util.Page');// 导入分页类
		$reverse = M('reverse_domain',Null,'DB_NEWS');
		$view = M('view',Null,'DB_NEWS');
		if(isset($_GET['v'])){
			$map["view_id"] = $_GET['v'];
			$this->assign('view',$_GET['v']);
		}
		if(!empty($_GET['ip'])){
			$map["ip"] = array('like','%'.$_GET['ip'].'%');
			$this->assign('ip',$_GET['ip']);
		}
			
		$count = $reverse->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$rlist = $reverse->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		$viewList = $view->select();
		foreach($rlist as $key => $val){
			$tem = $view->where('id='.$val['view_id'])->find();
			$rlist[$key]['view_name'] = $tem['name'];
		}
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录";
		}
		
		$this->assign('rlist', $rlist);// 赋值数据集
		$this->assign('viewList', $viewList);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();	
	}
	public function selectIP(){
		if(!empty($_GET['ip'])){			
			import('ORG.Util.Page');// 导入分页类
			$map["val"] = array('like','%'.$_GET['ip'].'%');
			$this->assign('ip',$_GET['ip']);
		
			$domain = M('domain',Null,'DB_NEWS');
			$view = M('view',Null,'DB_NEWS');
			$zone = M('zone',Null,'DB_NEWS');
			$reverse = M('reverse_domain',Null,'DB_NEWS');
			$count = $domain->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
			$Page = new Page($count, 20);// 实例化分页类 传入总记录数
			$nowPage = isset($_GET['p'])?$_GET['p']:1;
			$dlist = $domain->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
			foreach($dlist as $key => $val){
				$tem = $view->where('id='.$val['view'])->find();
				$dlist[$key]['view_name'] = $tem['name'];
				
				$tem2 = $zone->query('select * from client_domain cd left join zone z on z.id=cd.zone_id where cd.domain_id=%d',$val['id']);
				$dlist[$key]['domain'] = $val['host'].".".$tem2[0]['domain'];
				
			}
			$show = $Page->show();// 分页显示输出
			if(empty($show)){
				$show = " 0 条记录";
			}
			$this->assign('dlist', $dlist);
		}else{
			$show = " 0 条记录";
		}
		$this->assign('page',$show);
		$this->display();	
	}
	public function deleteReverse(){
		if(!empty($_POST['id']) && !empty($_POST['cid'])){
			$reverse = M('reverse_domain',Null,'DB_NEWS');			
			$data = $reverse->where('id='.$_POST['id'])->find();			
			$val = array("rid"=>(int)$_POST['id'], 'PTR'=>$data['domain']);
			$user = array("cid"=>$_POST['cid'], "level"=>0, "info"=>"");
			$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);			
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] != 0){
				$this->ajaxReturn('删除域名反解析失败，请联系管理员','error',0);
			}
			
			$is_ok = $reverse->where(array('id'=>array('in',$_POST['id'])))->delete();
			if($is_ok === false){
				$this->ajaxReturn('删除域名反解析失败，请联系管理员','error',0);
			}
			$this->ajaxReturn(1,'success',1);				
		}
	}	
	public function addReverse(){
		if($_SERVER['REQUEST_METHOD'] === 'GET'){				
			$view = M('view',Null,'DB_NEWS');
			$viewList = $view->select();
			$this->assign('viewList',$viewList);	
				
			$client = M('client',Null,'DB_NEWS');
			$clientList = $client->select();
			$this->assign('clientList',$clientList);
			
			$this->display();
		}else{
			if(!empty($_POST['ip']) && !empty($_POST['domain']) && !empty($_POST['view']) && !empty($_POST['client'])){			
				$reverse = M('reverse_domain',Null,'DB_NEWS');
				$entity = $reverse->where("ip='".$_POST['ip']."' and domain='".$_POST['domain']."' and view_id=".$_POST['view']." and client_id=".$_POST['client'])->find();
				if(!empty($entity)){
					$this->ajaxReturn('请不要重复添加相同的域名反解析','error',0);
				}
				$data['ip'] = $_POST['ip'];
				$data['domain'] = $_POST['domain'];
				$data['client_id'] = $_POST['client'];
				$data['view_id'] = $_POST['view'];
				$is_ok = $reverse->add($data);
				if($is_ok === false){
					$this->ajaxReturn('添加域名反解析失败，请联系管理员','error',0);
				}
				
				$val = array("name"=>strtolower($_POST['ip']), "rid"=>(int)$is_ok, 'PTR'=>rawurlencode($_POST['domain']), "ttl"=>(int)(10)*60, "viewid"=>$_POST['view']);
				$user = array("cid"=>$_POST['client'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn('添加域名反解析失败，请联系管理员','error',0);
				}
				
				$this->ajaxReturn(1,'success',1);
			}
		}
	}
	public function updateReverse(){
		if($_SERVER['REQUEST_METHOD'] === 'GET'){	
			if(!empty($_GET['id'])){
					
				$view = M('view',Null,'DB_NEWS');
				$viewList = $view->select();
				$this->assign('viewList',$viewList);	
					
				$client = M('client',Null,'DB_NEWS');
				$clientList = $client->select();
				$this->assign('clientList',$clientList);
				
				$reverse = M('reverse_domain',Null,'DB_NEWS');
				$ret = $reverse->where('id='.$_GET['id'])->find();				
				$this->assign('ret',$ret);
				$this->display();
			}					
		}else{
			if(!empty($_POST['id']) && !empty($_POST['ip']) && !empty($_POST['domain']) && !empty($_POST['view']) && !empty($_POST['client'])){
				$reverse = M('reverse_domain',Null,'DB_NEWS');
				$data['ip'] = $_POST['ip'];
				$data['domain'] = $_POST['domain'];
				$data['view_id'] = $_POST['view'];
				$data['client_id'] = $_POST['client'];
				$is_ok = $reverse->where('id='.$_POST['id'])->save($data);
				
				if($is_ok === false){
					$this->ajaxReturn('修改域名反解析失败，请联系管理员','error',0);
				}
				
				$val = array("name"=>strtolower($_POST['ip']), "rid"=>(int)$_POST['id'], 'PTR'=>rawurlencode($_POST['domain']), "ttl"=>(int)(10)*60, "viewid"=>$_POST['view']);
				$user = array("cid"=>$_POST['client'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,'修改域名反解析失败，请联系管理员',0);
				}
				$this->ajaxReturn(1,'success',1);
			}
		}
	}
	public function startStopReverse(){
		if(!empty($_POST['id']) && isset($_POST['status'])){
		$reverse = M('reverse_domain',Null,'DB_NEWS');
			$is_ok = $reverse->where(array('id'=>array('in',$_POST['id'])))->setField('status',$_POST['status']);			
			$rlist = $reverse->where(array('id'=>array('in',$_POST['id'])))->select();
			if($_POST['status']==0){
				if($is_ok === false){
					$this->ajaxReturn('启用域名反解析失败，请联系管理员','error',0);
				}				
				foreach($rlist as $r){
					$val = array("name"=>strtolower($r['ip']), "rid"=>(int)$r['id'], 'PTR'=>rawurlencode($r['domain']), "ttl"=>(int)(10)*60, "viewid"=>$r['view_id'],"enable"=>1);					
					$user = array("cid"=>$r['client_id'], "level"=>0, "info"=>"");
					$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}
				}
			}else{
				if($is_ok === false){
					$this->ajaxReturn('停用域名反解析失败，请联系管理员','error',0);
				}
				foreach($rlist as $r){
					$val = array("name"=>strtolower($r['ip']), "rid"=>(int)$r['id'], 'PTR'=>rawurlencode($r['domain']), "ttl"=>(int)(10)*60, "viewid"=>$r['view_id'],"enable"=>0);
					$user = array("cid"=>$r['client_id'], "level"=>0, "info"=>"");
					$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}
				}			
			}
			$this->ajaxReturn(1,'success',1);
		}
	}	
	
	/*public function returnList(){		
		$reverse = M('reverse_domain',Null,'DB_NEWS');
		$view = M('view',Null,'DB_NEWS');
		$rlist = $reverse->order('id desc')->select();
		foreach($rlist as $key => $val){
			$tem = $view->where('id='.$val['view_id'])->find();
			$rlist[$key]['view_name'] = $tem['name'];
		}
		
		return $rlist;
	}*/
}

