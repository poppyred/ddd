<?php
// 本类由系统自动生成，仅供测试用途
class DomainmgrAction extends BaseAction {

	public function index(){
		$list = M('zone',Null,'DB_NEWS'); 
		$client = M('client',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$map['level'] = array('lt','8');
		//非管理员
		if($_SESSION['level'] != 1){
			$map['internal'] = 0;
		}
		if(!empty($_GET['domain'])){
			$map["domain"] = array('like','%'.$_GET['domain'].'%');
			$this->assign('domain',$_GET['domain']);
		}
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$zlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录";
		}
		
		foreach($zlist as $key => $val){
			$user = $client->where("id=".$val['client_id'])->find();
			$zlist[$key]['mail'] = $user['mail'];
			$zlist[$key]['pwd'] = $user['pwd'];
		}
		$this->assign('zlist', $zlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	public function updateInternalDomain(){
		if(!empty($_POST['id'])){					
			$zone	= M('zone',Null,'DB_NEWS'); 			
			$is_ok = $zone->where('id='.$_POST['id'])->setField('internal',$_POST['internal']);
			if($is_ok === false){
				if($_POST['internal'] == 1){
					$this->ajaxReturn('设置内部域名失败，请联系管理员','error',0);
				}else{
					$this->ajaxReturn('设置外部域名失败，请联系管理员','error',0);
				}
			}
			if($_POST['internal'] == 1){
				$this->ajaxReturn("设置内部域名成功",'success',1);
			}else{
				$this->ajaxReturn("设置外部域名成功",'success',1);
			}
		}
	}
	public function addDomain(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){			
			$client = M('client',Null,'DB_NEWS'); 
			$list = $client->select();
			$this->assign("list",$list);
			$this->display();		
		}else{
			if(isset($_POST['level']) && !empty($_POST['zone']) && !empty($_POST['mail'])){
				$data = file_get_contents(C('API_URL')."/domain.php?opt=insert&user=".$_POST['mail']."&domain=".$_POST['zone']."&level=".$_POST['level']);
				$result = json_decode($data,true);
				if($result['ret'] != 0){
					$this->ajaxReturn($result['error'],'error',0);
				}
				$this->ajaxReturn("添加域名成功",'success',1);
			}
		}
	}
	public function transfer(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
		
			$client = M('client',Null,'DB_NEWS'); 
			$list = $client->where("mail!='".$_GET['cid']."'")->select();
			
			$this->assign('list', $list);	
			$this->assign('id', $_GET['id']);	
			$this->display();
			
		}else{
			$client = M('client',Null,'DB_NEWS'); 
			$zone = M('zone',Null,'DB_NEWS'); 
			$client_domain = M('client_domain',Null,'DB_NEWS'); 
			//$client_group = M('client_group',Null,'DB_NEWS'); 
			$vo = $zone->where('id='.$_POST['id'])->find();
			if($vo){
				//修改client_id有关数据
				$is_ok = $client_domain->where('zone_id='.$_POST['id'])->setField('client_id',$_POST['mail']);
				//$is_ok2 = $client_group->where('client_id='.$_POST['cid'])->setField('client_id',$_POST['mail']);
				$is_ok3 = $zone->where('id='.$_POST['id'])->setField('client_id',$_POST['mail']);
				if($is_ok===false || $is_ok3===false){
					$this->ajaxReturn('域名授权失败，请联系管理员','error',0);
				}else{
					$this->ajaxReturn($vo['level'],'success',1);
				}
			}
		}
		
	}
	
	public function empower(){
		if(isset($_POST['vip']) && !empty($_POST['domain'])){
			
			$data = file_get_contents(C('API_URL')."/domain.php?opt=empower&domain=".$_POST['domain']."&vip=".$_POST['vip']);			
			$result = json_decode($data,true);
			if($result['ret']!=0){				
				$this->ajaxReturn('域名授权失败，请联系管理员！','error',0);
			}
			$this->ajaxReturn(1,'success',1);
		}
	}
	public function mgrClient(){
		$this->assign('mail',$_GET['mail']);
		$this->assign('pwd',$_GET['pwd']);
		$this->assign('domain',$_GET['domain']);
		$this->display();	
	}
	
	public function flowchar(){
		if(!empty($_GET['r'])){
			
			$zone = M('zone',Null,'DB_NEWS'); 
			$tem = $zone->where('id='.$_GET['r'])->find();
			$domain = M('domain',Null,'DB_NEWS'); 
			$hostlist = $domain->query("select distinct(d.host) from client_domain cd left join domain d on cd.domain_id=d.id where cd.client_id=%d and zone_id=%d and d.type!='NS' order by d.host asc",$tem['client_id'],$tem['id']);
			$this->assign('hostlist', $hostlist);
			
			$type = "main";//默认域名
			$timezone = "today";//默认今天
			
			if(!empty($_GET['type'])){
				$type = $_GET['type'];
			}
			if(!empty($_GET['timezone'])){
				$timezone = $_GET['timezone'];
			}
			
			$data = file_get_contents(C('NS_CHECH_URL')."/script/eflydns_client_reqcnt_chart.php?type=".$type."&domain=".$_GET['host'].".".$_GET['d']."&timezone=".$timezone);
			$list = json_decode($data,true);
			
			if($list['ret']==0){
				foreach($list['descmap'] as $key => $val){
					$time .= $key . ",";
					$num .= $val .",";
					$sum += $val;
				}
			}
			
			$this->assign('host', $_GET['host']);
			$this->assign('time', trim($time,","));
			$this->assign('num', trim($num,","));
			$this->assign('type', $type);
			$this->assign('sum', $sum);
			$this->assign('timezone', $timezone);
			$this->assign('zone', $tem['domain']);
			$this->assign('r',$_GET['r']);
			$this->display();	
		}
	}
	public function vip(){
		$list = M('zone',Null,'DB_NEWS'); 
		$client = M('client',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$map['level'] = array('egt','8');
		//非管理员
		if($_SESSION['level'] != 1){
			$map['internal'] = 0;
		}
		if(!empty($_GET['domain'])){
			$map["domain"] = array('like','%'.$_GET['domain'].'%');
			$this->assign('domain',$_GET['domain']);
		}
		$count = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$zlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		//print_r($list->getlastsql());exit;
		$show = $Page->show();// 分页显示输出
		if(empty($show)){
			$show = " 0 条记录";
		}
		foreach($zlist as $key => $val){
			$user = $client->where("id=".$val['client_id'])->find();
			$zlist[$key]['mail'] = $user['mail'];
			$zlist[$key]['pwd'] = $user['pwd'];
		}
		$this->assign('zlist', $zlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function readZone(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$tb = M('zone',Null,'DB_NEWS'); 
			$ret = $tb->where('id='.$_POST['id'])->find();
			//print_r($tb->getlastsql());exit;
			$client = M('client',Null,'DB_NEWS'); 
			$user = $client->where('id='.$ret['client_id'])->find();
			$ret['user'] = $user['mail'];
			$this->ajaxReturn($ret,'',1);
		}
	}
	public function startOrStop(){
		if(!empty($_POST['id'])){
			$zone = M('zone',Null,'DB_NEWS'); 
			$tem = $zone->where('id='.$_POST['id'])->find();
			$client = M('client',Null,'DB_NEWS'); 
			$user = $client->where('id='.$tem['client_id'])->find();
			$data = file_get_contents(C('API_URL')."/domain.php?opt=updateStatus&user=".$user['mail']."&domain=".$tem['domain']."&status=".$_POST['status']);			
			$result = json_decode($data,true);
			if($result['ret']==0){
				$this->ajaxReturn('域名'.$_POST['status'].'成功！','success',1);
			}else{
				$this->ajaxReturn('域名'.$_POST['status'].'失败！','error',0);
			}
		}
	}
	public function delDomain(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			if(!empty($_POST['id'])){
				$zone = M('zone',Null,'DB_NEWS'); 
				$tem = $zone->where('id='.$_POST['id'])->find();
				//删除接口
				$data = file_get_contents(C('API_URL')."/domain.php?opt=delete&domain=".$tem['domain']);
				$result = json_decode($data,true);
				if($result['ret']==0){
					$this->ajaxReturn('域名已经删除！','success',1);
				}else{
					$this->ajaxReturn('域名删除失败！','error',0);
				}
			}
		}
	}
	
	public function record(){
		if(!empty($_GET['r'])){
			import('ORG.Util.Page');// 导入分页类
			$client_domain = M('client_domain',Null,'DB_NEWS'); 
			$domain = M('domain',Null,'DB_NEWS'); 
			$tem = $client_domain->where('zone_id='.$_GET['r'])->getField('domain_id',true);
			$str;
			foreach($tem as $val){
				$str .= $val . "," ;
			}
			$str = substr($str,0,-1);
			$map['id'] = array('in',$str);
			if(!empty($_GET['val'])){
				$map['val'] = array('like','%'.$_GET['val'].'%');
				$this->assign('val',$_GET['val']);
			}
			if(!empty($_GET['type'])){
				$map['type'] = $_GET['type'];
				$this->assign('type',$_GET['type']);
			}
			if(!empty($_GET['view'])){
				$map['view'] = $_GET['view'];
				$this->assign('view',$_GET['view']);
			}
			$dlist = $domain->where($map)->select();
			
			
			$Page = new Page(count($dlist), 20);// 实例化分页类 传入总记录数
			$nowPage = isset($_GET['p'])?$_GET['p']:1;
			$show = $Page->show();// 分页显示输出
			if(empty($show)){
				$show = " 0 条记录";
			}
			
			$view = M('view',Null,'DB_NEWS'); 
			$vlist = $view->select();
			$this->assign('vlist',$vlist);
			
			$userId = $client_domain->where('zone_id='.$_GET['r'])->getField('client_id');
			$this->assign('userId',$userId);
			
			$zone = M('zone',Null,'DB_NEWS'); 
			$level = $zone->where('id='.$_GET['r'])->getField('level');
			$this->assign('level',$level);	
			
			$this->assign('r',$_GET['r']);	
			$this->assign('dlist',$dlist);		
			$this->assign('page',$show);// 赋值分页输出	
			$this->display();		
		}
	}
	public function deleteRecord(){
		if(!empty($_POST['id'])){
			$domain = M('domain',Null,'DB_NEWS'); 
			$data = $domain->where('id='.$_POST['id'])->find();
			
			$val = array("rid"=>(int)$_POST['id'], $data['type']=>$data['val']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			
			//别名删除
			$zone = M('zone',Null,'DB_NEWS');
			$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.id=".$_POST['zid']);
			if(!empty($bmlist)){
				foreach($bmlist as $value){
					$data = $domain->where('id='.$_POST['id'])->find();
					$val = array("rid"=>(int)$_POST['id'] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>$data['val']);
					$user = array("cid"=>$_POST['userId'], "level"=>0, "info"=>"");
					$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["content"],"error",0);	
					}
				}
			}
			
			$client_domain = M('client_domain',Null,'DB_NEWS');
			$is_ok = $client_domain->where('domain_id='.$_POST['id'].' and zone_id='.$_POST['zid'])->delete();
			$is_ok2 = $domain->where('id='.$_POST['id'])->delete();
			if($is_ok === false || $is_ok2 ===false){
				$this->ajaxReturn('删除记录失败。','error',0);	
			}
			$this->ajaxReturn(1,'success',1);	
		}	
	}
	public function selectDomainCount($id){
		$client_domain = M('client_domain',Null,'DB_NEWS');
		$count = $client_domain->where('zone_id='.$id)->count();
		return $count;
	}
	public function addRecord(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$view = M('view',Null,'DB_NEWS'); 
			$vlist = $view->select();			
			$this->assign('vlist',$vlist);			
			$this->assign('zid',$_GET['zid']);
			$this->display();			
		}else{
			$domain = M('domain',Null,'DB_NEWS');		
			$zone = M('zone',Null,'DB_NEWS');
			$client_domain = M('client_domain',Null,'DB_NEWS');
			
			$vo = $zone->where('id='.$_POST['zid'])->select();
			if($vo[0]['level']==0){
				if($this->selectDomainCount($vo[0]['id']) == 10){
					$this->ajaxReturn(0,'记录已经上限，最多10条解析记录',0);
				}
			}else{
				if($this->selectDomainCount($vo[0]['id']) == 50){
					$this->ajaxReturn(0,'记录已经上限，最多50条解析记录',0);
				}
			}
			
			$entity = $domain->query("select * from domain d left join client_domain cd on d.id=cd.domain_id where host='".$_POST['host']."' and type='".$_POST['type']."' and view=".$_POST['view']." and val='".$_POST['val']."' and mx=".$_POST['mx']." and cd.zone_id=".$vo[0]['id']." and cd.client_id=".$vo[0]['client_id']);
			
			if(!empty($entity)){
				$this->ajaxReturn(0,'请不要添加相同的解析记录',0);
			}
			
			$data['host'] = $_POST['host'];
			$data['type'] = $_POST['type'];
			$data['view'] = $_POST['view'];
			$data['val'] = $_POST['val'];
			$data['mx'] = $_POST['mx'];
			$data['ttl'] = $_POST['ttl'];
			$data['desc'] = '';
			//////
			$is_ok = $domain->add($data);
			if($is_ok){
				//域名添加记录
				$val = array("name"=>strtolower($data['host']).".".$vo[0]['domain'], "main"=>$vo[0]['domain'], "rid"=>(int)$is_ok, $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
				$user = array("cid"=>$vo[0]['client_id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				//别名添加记录
				$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.domain='".$vo[0]['domain']."'");
				if(!empty($bmlist)){
					foreach($bmlist as $val){
						$val = array("name"=>strtolower($data['host']).".".$val['name'], "main"=>$val['name'], "rid"=>(int)$is_ok + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
						$user = array("cid"=>$vo[0]['client_id'], "level"=>0, "info"=>"");
						$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
						$rslt = json_decode($ret["content"],true);
						if($rslt["ret"] != 0){
							$this->ajaxReturn(0,'添加失败，联系管理员',0);
						}
					}
				}
				if($rslt["ret"] == 0){
					//添加client_domain信息
					$data1['client_id'] = $vo[0]['client_id'];
					$data1['zone_id'] = $vo[0]['id'];
					$data1['domain_id'] = $is_ok;
					$client_domain->add($data1);
					$this->ajaxReturn($is_ok,'解析记录添加成功',1);
				}
			}else{
				$this->ajaxReturn(0,'添加失败，联系管理员',0);
			}
		}	
	}
	
	public function editRecord(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$domain = M('domain',Null,'DB_NEWS'); 
			$view = M('view',Null,'DB_NEWS'); 
			$data = $domain->where('id='.$_GET['id'])->find();
			$vlist = $view->select();
			
			$this->assign('vlist',$vlist);
			$this->assign('userId',$_GET['userId']);
			$this->assign('domain',$data);
			$this->assign('zid',$_GET['zid']);
			$this->display();
		}else{
			if(isset($_POST['id']) && isset($_POST['host']) && isset($_POST['type']) && isset($_POST['view']) && isset($_POST['mx']) && isset($_POST['val']) && isset($_POST['ttl'])){
				$domain = M('domain',Null,'DB_NEWS'); 
				$entity = $domain->query("select * from domain d left join client_domain cd on d.id=cd.domain_id where host='".$_POST['host']."' and type='".$_POST['type']."' and view=".$_POST['view']." and val='".$_POST['val']."' and mx=".$_POST['mx']." and cd.client_id=".$_POST['userId']." and cd.zone_id=".$_POST['zid']." and d.id not in (".$_POST['id'].") ");
				if(!empty($entity)){		
					$this->ajaxReturn('请不要修改相同的解析记录。','error',0);		
				}
				$data['host'] = $_POST['host'];
				$data['type'] = $_POST['type'];
				$data['view'] = $_POST['view'];
				$data['mx'] = $_POST['mx'];
				$data['val'] = $_POST['val'];
				$data['ttl'] = $_POST['ttl'];
				
				
				$zone = M('zone',Null,'DB_NEWS'); 
				$z = $zone->where('id='.$_POST['zid'])->find();
				
				$val = array("name"=>$data['host'].".".$z['domain'], "main"=>$z['domain'], "rid"=>(int)$_POST['id'], $data['type']=>rawurlencode($data['val']), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
				$user = array("cid"=>$_POST['userId'], "level"=>0, "info"=>"");		
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);		
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);		
				$rslt = json_decode($ret["content"],true);
				
				$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.id=".$_POST['zid']);
				if(!empty($bmlist)){	
					foreach($bmlist as $value){		
						$val = array("name"=>$data['host'].".".$val['name'], "main"=>$val['name'], "rid"=>(int)$_POST['id'] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>rawurlencode($data['val']), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);		
						$user = array("cid"=>$_POST['userId'], "level"=>0, "info"=>"");		
						$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);		
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);		
						$rslt = json_decode($ret["content"],true);		
						if($rslt["ret"] != 0){		
							$this->ajaxReturn($rslt["error"],'error',0);		
						}		
					}		
				}
				
				$is_ok = $domain->where('id='.$_POST['id'])->save($data);
				if($is_ok === false){
					$this->ajaxReturn('修改解析记录失败','error',0);
				}
				$this->ajaxReturn('修改解析记录成功','success',1);
			}
		}
	}
}
