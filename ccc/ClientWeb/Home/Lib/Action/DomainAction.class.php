<?php
// 本类由系统自动生成，仅供测试用途
class DomainAction extends BaseAction {
	
	//域名解析记录
	public function detail(){
		if(!empty($_GET['d'])){
			$client_domain = M("client_domain");
			$z = M("zone");
			$domain = M("domain");
			$zone = $z->where('client_id='.$_SESSION['id'].' and  domain="'.$_GET['d'].'"')->find();
			if(!empty($zone)){
				//查询是否存在可以编辑的记录
				$rcd = $client_domain->query('select * from client_domain a left join domain b on b.id=a.domain_id  where a.zone_id=%d and client_id=%d and is_edit=1',$zone["id"],$_SESSION['id']);  
				//$rcd = $client_domain->query('select * from client_domain a left join domain b on b.id=a.domain_id  where a.zone_id=%d and client_id=%d and b.type!=\'NS\'',$zone["id"],$_SESSION['id']);
				if(empty($rcd)){ //没有可编辑的记录
					$this->assign('is_new', "true");
				}
			
				$tem = $z->where('id='.$zone["id"])->select();
				$this->assign('level', $tem[0]['level']);
				//$this->assign('is_on', $tem[0]['is_on']);
				$view = M('view');
				$viewList = $view->select();
				$this->assign('viewList', $viewList);
				$page = 1;
				if(!empty($_GET['p'])){
					$page = $_GET['p'];
				}
				$orderBy = " order by a.id desc";
				if(!empty($_GET['o'])){
					$orderBy = $_GET['o'];
				}
				//$dmList = $domain->where(array("cd.zone_id"=>$vo['id'],"cd.client_id"=>$_SESSION['id'],"d.host"=>array("like","%".$_POST['val']."%")))->query("select * from client_domain cd left join domain d on cd.domain_id=d.id %WHERE%",true);
				$where = "";
				if(!empty($_GET['t'])){
					$where = " and b.type='".$_GET['t']."' ";
				}
				if(!empty($_GET['h'])){
					$where .= " and b.host like '%".$_GET['h']."%' ";
				}
				if(!empty($_GET['v'])){
					$where .= " and b.view=" . $_GET['v'] . " ";
				}
			
				$record = $domain->query('select * from client_domain a left join domain b on b.id=a.domain_id where a.zone_id='.$zone["id"].' and client_id='.$_SESSION['id'].' '. $where . $orderBy . ' limit '. ($page-1) * 50 .',50');
				$count = $domain->query('select count(*) as count from client_domain a left join domain b on b.id=a.domain_id where a.zone_id='.$zone["id"].' and client_id='.$_SESSION['id'] . $where . '  order by a.id desc');
				if($count[0]['count']<=50){
					$pageCount = 1;
				}else{
					if($count[0]['count'] % 50 == 0){
						$pageCount = $count[0]['count'] / 50;
					}else{
						$pageCount = ($count[0]['count'] - $count[0]['count'] % 50) / 50 + 1;
					}
				}
				$this->assign('sum',$count[0]['count']);
				$this->assign('type',$_GET['t']);
				$this->assign('host',$_GET['h']);
				$this->assign('view',$_GET['v']);
				$this->assign('orderBy',$orderBy);
				$this->assign('page',$page);
				$this->assign('recordlist', $record);
				$this->assign('pageCount',$pageCount);
				$this->assign('is_lock',$zone['is_lock']);
				$this->assign('default_ttl',$zone['default_ttl']);
				$this->assign('zone', $_GET['d']);
				$this->assign('count', count($record));					
				$this->display();
			}else{
				header("Location: ".__APP__."/Domain/domainList");	
			}
		}
	}
	
	public function searchRecord(){
		if(!empty($_POST['rcd'])){
			//查找record记录过程
			try {
				header("Content-Type:text/html;charset=utf-8");
				/*
				* 指定WebService路径并初始化一个WebService客户端
				*/
				if (!class_exists('SoapClient')){
					$this->ajaxReturn(json_decode("You haven't installed the PHP-Soap module."),'error',0);
				}
				$ws = C('SERVICE_URL')."/dnspro_soap/services/record?wsdl";//webservice服务的地址
				//print_r($ws);exit();
				$client = new SoapClient($ws);
				//$client = new SoapClient(null,array('location'=>'/dnspro_soap/services',uri => 'record'));
				/*
				* 获取SoapClient对象引用的服务所提供的所有方法
				*/
				//var_dump ($client->__getFunctions());//获取服务器上提供的方法
				//var_dump ($client->__getTypes());//获取服务器上数据类型

				$result = $client->get_domain_records($_POST['rcd']);//查询
				$this->ajaxReturn(json_decode($result),'success',1); //json_decode()
			} catch (SOAPFault $e) {
				$this->ajaxReturn($e,'error',0);
			}
		}
	}
	public function findDomain(){
		if(!empty($_POST['zoneID'])){
			$domain = M('domain');
			$domainList = $domain->query('select * from client_domain cd left join domain d on cd.domain_id=d.id where cd.zone_id=%d and cd.client_id=%d',$_POST['zoneID'],$_SESSION['id']);
			if($domainList){
				$this->ajaxReturn($domainList,'success',1);	
			}else{
				$this->ajaxReturn('单组记录绑定失败，请联系管理员','error',0);	
			}
		}
	}
	public function addMessage(){
		if(!empty($_POST['zone']) && !empty($_POST['phone']) && !empty($_POST['mail']))	{
			$zone = M('zone');
			$message = M('message');
			$vo = $zone->where("domain='".$_POST['zone']."'")->find();
			$data['zone_id'] = $vo['id'];
			$data['client_id'] = $_SESSION['id'];
			$data['phone'] = $_POST['phone'];
			$data['mail'] = $_POST['mail'];
			$is_ok = $message->add($data);
			if($is_ok === false){
				$this->ajaxReturn('开启实时通知失败，请联系管理员。','error',0);
			}
			$this->ajaxReturn(1,'success',1);
		}
	}
	public function closeMsg(){
		if(!empty($_POST['zone'])){
			$zone = M('zone');
			$message = M('message');
			$vo = $zone->where("domain='".$_POST['zone']."'")->find();
			$is_ok = $message->where('client_id='.$_SESSION['id'].' and zone_id='.$vo['id'])->delete();
			if($is_ok === false){
				$this->ajaxReturn('关闭实时通知失败，请联系管理员。','error',0);
			}
			$this->ajaxReturn(1,'success',1);
		}
	}
	
	public function domainSet(){
		if(!empty($_GET['d'])){
			$z = M("zone");
			$cd = M('client_domain');
			$client = M('client');
			$message = M('message');
			$zone = $z->where('domain="'.$_GET['d'].'"')->find();
			$l = M("zone_level");
			$level = $l->where('zone_id="'.$zone['id'].'"')->find();	
			$tem = $cd->query('select * from client_domain cd left join client c on cd.client_id=c.id where zone_id=%d group by client_id ',$zone['id']);
			
			$vo = $message->where('client_id='.$_SESSION['id'].' and zone_id='.$zone['id'])->find();
			if(empty($vo)){				
				$this->assign('status', 0);		
			}else{
				$this->assign('status', 1);		
			}
			$client = M('client');
			$entity = $client->where('id='.$_SESSION['id'])->find();
			$this->assign('entity', $entity);
			$this->assign('user', $tem);
			$this->assign('userCount', count($tem));
			$this->assign('level', $level);			
			//$this->assign('is_on', $zone['is_on']);
			$this->assign('zone', $zone);
			$this->display();
		}
	}
	public function domainRpt(){
		if(!empty($_GET['d'])){
			$z = M("zone");
			$zone = $z->where('domain="'.$_GET['d'].'"')->find();
			$l = M("zone_level");
			$level = $l->where('zone_id="'.$zone['id'].'"')->find();
			
			
			$domain = M("domain");
			$hostlist = $domain->query("select distinct(d.host) from client_domain cd left join domain d on cd.domain_id=d.id where cd.client_id=%d and zone_id=%d and d.type!='NS' order by d.host asc",$_SESSION['id'],$zone['id']);
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
			
			$this->assign('level', $level);
			//$this->assign('is_on', $zone['is_on']);
			$this->assign('zone', $_GET['d']);
			$this->display();
		}
	}
	public function domainRoute(){
		if(!empty($_GET['d'])){
			$z = M("zone");
			$route = M('route');
			$route_mask = M('route_mask');
			$zone = $z->where('domain="'.$_GET['d'].'"')->find();
			$l = M("zone_level");
			$level = $l->where('zone_id="'.$zone['id'].'"')->find();
			$route_mask_list = $route_mask->query('select r.*,rm.id,rm.route_id,INET_NTOA(rm.mask_start) as mask_start,INET_NTOA(rm.mask_end) as mask_end from route r left join route_mask rm on r.id=rm.route_id where r.zone_id=%d',$zone['id']);
			$route_list = $route->query('select * from route where zone_id=%d group by name order by id desc ',$zone['id']);
			
			$this->assign('level', $level);			
			$this->assign('rlist_count', count($route_list));
			$this->assign('rlist', $route_list);
			$this->assign('rmlist', $route_mask_list);
			//$this->assign('is_on', $zone['is_on']);
			$this->assign('zone', $_GET['d']);
			$this->assign('zone_id', $zone['id']);
			$this->display();
		}
	}
	
	public function recordList(){
		if(!empty($_GET['d'])){
			//print_r(json_decode(json_encode($_GET['d'])));
			$data = object_array(json_decode($_GET['d'])); //
			$this->assign('recordlist',$data);
			//print_r($data);
			$this->assign('recordnum',count($data));
			$this->display();
		}
	}
	//间隔扫描
	public function refreshStatus(){			
		$list = M("client");
		$ret = $list->where("mail='".$_SESSION['user']."'")->find();
		if(!empty($ret)){		
			//$last_login = strtotime($ret["login_time"]);
			$data["login_time"] = date("Y-m-d H:i:s");
			$span = floor((strtotime($data["login_time"]) - $last_login) / 60); //min
			if($span >= 5){
				//$param = array("id"=>$ret["id"]);
				//http_post(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php", $param);
				file_get_contents(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php?domain=".$_POST['zone']);
				//print(C('NS_CHECH_URL')."/script/eflydns_client_domain_check.php?id=".$ret["id"]);exit;	
			}
//			print(123);
			$this->ajaxReturn(1,'success',1);
		}	
	}
	//域名记录
	public function domainList(){
		$zone = M();
		if(!empty($_GET['g']) && !empty($_SESSION['id'])){
			if($_GET['g'] == 'all'){
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.client_id=%d group by z.id order by z.is_author desc',$_SESSION['id']);
			}else if($_GET['g'] == 'star'){
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.client_id=%d and z.is_star=1 group by z.id order by z.id desc',$_SESSION['id']);
			}else if($_GET['g'] == 'recent'){
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.client_id=%d and z.is_author=1 group by z.id order by z.id desc',$_SESSION['id']);
			}else if($_GET['g'] == 'vip'){
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain where client_id=%d group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.level>0 and cd.client_id=%d group by z.id order by z.id desc ',$_SESSION['id'],$_SESSION['id']);
			}else if($_GET['g'] == 'error'){
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.client_id=%d and z.is_author=0 group by z.id order by z.id desc',$_SESSION['id']);
			}else{
				$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd  on cd.zone_id=z.id where z.client_id=%d and z.group_id=%d group by z.id order by z.id',$_SESSION['id'],$_GET['g']);
			}
			$this->assign('group',$_GET['g']);
		}else{
			$zoneList = $zone->query('select * from (select * from zone a left join (select zone_id,count(*) as count from client_domain group by zone_id ) as b on a.id=b.zone_id) as z left join client_domain cd on cd.zone_id=z.id where z.client_id=%d group by z.id order by z.is_author desc ',$_SESSION['id']);//limit 0,10
			$this->assign('group','recent');
		}
		$name = M('zone_name');
		foreach($zoneList as $n => $zone){
			$cnd['zone_id'] = $zone["zone_id"];
			$zoneList[$n]["zone_name"] = $name->where($cnd)->select();
			//print_r($name->getlastsql());exit;
		}
		if(!empty($_GET['nowtime'])){
			$this->assign('nowtime',$_GET['nowtime']);		
		}
		$group = M('client_group');
		$glist = $group->where('client_id='.$_SESSION['id'])->select();
		$this->assign('grouplist',$glist);		
		$this->assign('zoneListCount',count($zoneList));
		$this->assign('zoneList',$zoneList);
		$this->display();
	}

	//验证域名是否存在
	public function checkZone(){
		if(!empty($_POST['zone'])){
			$zone = M('zone');
			$vo = $zone->where("domain='".$_POST['zone']."'")->find();
			if($vo){
				if($vo['client_id']==$_SESSION['id']){
					$this->ajaxReturn('域名已存在于解析列表中，无需再次添加。点击解析列表中的域名即可开始解析。','error',0);	
				}else{
					$this->ajaxReturn('域名被其他账户添加。若您所属域名因被别人抢占而导致添加失败，请点击右下方按钮与客服联系进行域名拥有者身份验证。','error',0);	
				}
		   	}else{
				$this->ajaxReturn($vo,'success',1);
		   	}
		}
	}
	//查询用户的域名个数
	public function selectZoneCount(){
		$zone = M('zone');
		$count = $zone->where('client_id='.$_SESSION['id'])->count();
		return $count;
	}
	
	//添加域名
	public function addZone(){
		if(!empty($_POST['zone'])){
			if(!preg_match('/^([\w-]+\.)+((com)|(net)|(org)|(gov\.cn)|(info)|(cc)|(com\.cn)|(net\.cn)|(org\.cn)|(name)|(biz)|(tv)|(cn)|(mobi)|(name)|(sh)|(ac)|(io)|(tw)|(com\.tw)|(hk)|(com\.hk)|(ws)|(travel)|(us)|(tm)|(la)|(me\.uk)|(org\.uk)|(ltd\.uk)|(plc\.uk)|(in)|(eu)|(it)|(jp)|(coop)|(edu)|(mil)|(int)|(ae)|(at)|(au)|(be)|(bg)|(br)|(bz)|(ca)|(ch)|(cl)|(cz)|(de)|(fr)|(hu)|(ie)|(il)|(ir)|(mc)|(to)|(ru)|(aero)|(nl))$/',$_POST['zone'])){ 
				$this->ajaxReturn(0,'域名格式不正确，请填写正确的域名(例如：baidu.com)。',0);
			}
			
			//普通用户限制50条
			$rowCount = $this->selectZoneCount();
			if($rowCount == C('ZONE_LIMIT')){
				$this->ajaxReturn(0,'域名已经上限，最多50条域名记录',0);
			}
			
			$zone = M('zone');
			$data['domain'] = $_POST['zone'];
			$data['client_id'] = $_SESSION['id'];
			 //type=domain&opt=add&data={"name":"test.com"}&user={"cid":87,"level":2,"info":""}
			$val = array("name"=>strtolower($data['domain']));
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"domain", "opt"=>"add", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] == 0){
				$is_ok = $zone->add($data);
				//普通用户, 添加两条默认NS记录
				$domain = M('domain');
				$client_domain = M('client_domain');	
				$ns = M('nameserver');
				$record = $ns->where('level=0')->select();
				foreach($record as $n => $value){
					$dt['host'] = '@';
					$dt['type'] = 'NS';
					$dt['view'] = $value['view'];
					$dt['route'] = 0;
					$dt['val'] = $value['val'];
					$dt['mx'] = 0;
					$dt['ttl'] = $value['ttl'];
					$dt['is_edit'] = 0;//$zone['is_author'];//如果域名是付费的，则是可以编辑，默认是不可编辑
					$dt['is_on'] = 1;
					$d_id = $domain->add($dt);
					
					$ds = array("name"=>$dt['host'].".".$dt['domain'], "main"=>$data['domain'], "rid"=>(int)$d_id, "domain_ns"=>$dt['val'], "level"=>$dt['mx'], "ttl"=>(int)($dt['ttl'])*60, "viewid"=>$dt['view']);
					$use = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$par = array("type"=>"record", "opt"=>"add", "data"=>$ds, "user"=>$use);
					$rt = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $par);
					$rs = json_decode($rt["content"], true);
					if($rs["ret"] != 0){ //添加失败
						$domain->where('id='.$d_id)->delete();
					}else{
						$rcd['client_id'] = $_SESSION['id'];
						$rcd['zone_id'] = $is_ok;
						$rcd['domain_id'] = $d_id;
						$client_domain->add($rcd);
					}
				}
				//zone添加成功
				if($is_ok){
					$zone_whois = M('zone_whois');
					$d["zone_id"] = $is_ok;
					$zone_whois->add($d);
					$this->ajaxReturn(1,'添加成功',1); //data,info,status
				}else{
					$this->ajaxReturn(0,'添加失败，联系管理员',0);
				}
			}else{
				$this->ajaxReturn(0,$rslt["error"],0);
			}
		}
	}
	public function updateZoneTime($id){
		$zone = M('zone');
		date_default_timezone_set('PRC');
		$zone->where('id='.$id)->setField('edit_time',date('Y-m-d H:i:s',time()));
	}
	//查找备注
	public function findZoneDesc(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$vo = $zone->where("domain='".$_POST['domain']."'")->select();
			if($vo){
				$this->ajaxReturn($vo,'success',1);
			}else{
				$this->ajaxReturn(0,'查找备注失败,请联系管理员',1);
			}			
		}
	}
	//添加,修改备注
	public function addZoneDesc(){
		if(!empty($_POST['domain'])){			
			$zone=M('zone');
			//$data = array('description'=>);
			$zone->where('domain="'.$_POST['domain'].'"')->setField('desc',$_POST['desc']);
			$this->ajaxReturn($zone,'success',1);
		}
	}
	//删除域名
	public function deleteZone(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$zone_name = M('zone_name');
			$domain = M('domain');
			$client_domain = M('client_domain');
			$arr = explode(',',trim($_POST['domain']));
			$del_rslt = true;
			for($i=0;$i<count($arr);$i++){
				$temp = $zone->where('domain="'.$arr[$i].'"')->select();
				//删除域名
				$val = array("name"=>strtolower($arr[$i]));
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"domain", "opt"=>"del", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] == 0){
					$temp2 = $client_domain->where('zone_id='.$temp[0]['id'])->select();
					for($j=0;$j<count($temp2);$j++){
						//删除记录
						$data = $domain->where('id='.$temp2[$j]['domain_id'])->find();						
						$val = array("rid"=>(int)$data['id'], $data['type']=>strtolower($data['val']));
						$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
						$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
						$rslt = json_decode($ret["content"],true);
						if($rslt["ret"] != 0){
							$this->ajaxReturn(0,'域名删除失败，请联系管理员',0);
						}
							
						$domain->where('id='.$temp2[$j]['domain_id'])->delete();
					}
					$client_domain->where('zone_id='.$temp[0]['id'])->delete();
				}else{
					$del_rslt = false;
				}
			}
			//删除 route route_mask
			for($i=0;$i<count($arr);$i++){
				$temp = $zone->where('domain="'.$arr[$i].'"')->find();
				$route = M('route')->where('zone_id='.$temp['id'])->find();
				M('route_mask')->where('route_id='.$route['id'])->delete();
				M('route')->where('zone_id='.$temp['id'])->delete();
			}
			//删除zone_level
			for($i=0;$i<count($arr);$i++){
				$temp = $zone->where('domain="'.$arr[$i].'"')->find();
				M('zone_level')->where('zone_id='.$temp['id'])->delete();
			}
			//删除zone_whois
			for($i=0;$i<count($arr);$i++){
				$temp = $zone->where('domain="'.$arr[$i].'"')->find();
				M('zone_whois')->where('zone_id='.$temp['id'])->delete();
			}
			//删除别名
			for($i=0;$i<count($arr);$i++){
				$temp = $zone->where('domain="'.$arr[$i].'"')->find();
				$list = $zone_name->where('zone_id='.$temp['id'])->select();				
				foreach($list as $value){
					$val = array("name"=>$value['name']);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"domain", "opt"=>"del", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					
					if($rslt["ret"] != 0){
						$this->ajaxReturn(0,'删除失败，联系管理员',0);
					}
				}
				
				$is_ok = $zone_name->where('zone_id='.$temp['id'])->delete();
				if($is_ok===false){
					$this->ajaxReturn(0,'域名删除失败，请联系管理员',0);
				}			
			}
			if($del_rslt){
				$is_ok = $zone->where(array('domain'=>array('in',$_POST['domain'])))->delete();
				if($is_ok == false){
					$this->ajaxReturn(0,'域名删除失败，请联系管理员',0);
				}else{
					$this->ajaxReturn($zone,'success',1);
				}
			}else{
				$this->ajaxReturn(0,'域名删除失败，请联系管理员',0);
			}
		}
	}
	//添加,取消标记
	public function maskStar(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$zone->where('domain="'.$_POST['domain'].'"')->setField('is_star',$_POST['is_star']);
			$this->ajaxReturn($zone,'success',1);
		}
	}
	//批量添加域名
	public function batckAddZone(){
		if(!empty($_POST['str'])){
			$arr = explode(',',trim($_POST['str']));
			$data;
			for($i=0;$i<count($arr);$i++){
				if(!empty($arr[$i])){
					$client_domain = M('client_domain');	
					$zone=M('zone');
					$data[$i]['domain'] = $arr[$i];
					$data[$i]['client_id'] = $_SESSION['id'];
					$data[$i]['desc'] = '';
					$is_ok = $zone->add($data[$i]);
					//添加client_domain信息
					
					$data1['client_id']=$_SESSION['id'];
					$data1['zone_id']=$is_ok;
					$client_domain->add($data1);
						
					if($is_ok==false){
						$this->ajaxReturn(0,'添加失败，联系管理员',0);
					}
				}
			}
			$this->ajaxReturn($data,'success',1);
		}
	}	
	//查看域名的别名
	public function lookBiem(){
		if(!empty($_POST['domain'])){
//			print_r($_POST['domain']);
			$zone = M('zone');
			$name = M('zone_name');
			$vo = $zone->where('domain=\''.$_POST['domain'].'\'')->find();
			//print_r($zone->getLastSQL());
			$nameList = $name->query('select * from `zone_name` where zone_id="%d" order by id desc',$vo['id']);
			//print_r($name->getLastSQL());
			$this->ajaxReturn($nameList,'success',1);
		}	
	}
	//排序解析记录
	public function sortDomain(){
		if(!empty($_POST['zone'])){
			$zone = M('zone');
			$vo = $zone->where('domain=\''.$_POST['zone'].'\'')->find();
			$domain = M('domain');
			if(!empty($_POST['p'])){
				$page = $_POST['p'];
			}
			$where = "";
			if(!empty($_POST['t'])){
				$where = " and b.type='".$_POST['t']."' ";
			}
			if(!empty($_POST['h'])){
				$where .= " and b.host like '%".$_POST['h']."%' ";
			}
			if(!empty($_POST['v'])){
				$where .= " and b.view=" . $_POST['v'] . " ";
			}
			
			$record = $domain->query('select * from client_domain a left join domain b on b.id=a.domain_id where a.zone_id=%d and client_id=%d '. $where .' %s limit '. ($page-1) * 50 .',50',$vo["id"],$_SESSION['id'], $_POST['ord']);
			$this->ajaxReturn($record,'success',1);
		}
	}
	//查找解析记录备注
	public function findDescById(){
		if(!empty($_POST['id'])){
			$domain	= M('domain');
			$vo = $domain->where('id='.$_POST['id'])->select();
			$this->ajaxReturn($vo,'success',1);
		}
	}
	//添加解析记录备注
	public function addDomainDesc(){
		if(!empty($_POST['id'])){
			$domain = M('domain');
			$domain->where('id='.$_POST['id'])->setField('desc',$_POST['desc']);
			$this->ajaxReturn($domain,'success',1);
		}
	}
	
	public function selectDomainCount($id){
		$client_domain = M('client_domain');
		$count = $client_domain->where('zone_id='.$id)->count();
		return $count;
	}
	//添加解析记录
	public function addDomain(){
		$domain = M('domain');		
		$zone = M('zone');
		$client_domain = M('client_domain');
		$vo = $zone->where('domain="'.$_POST['zone'].'"')->select();
		if($vo[0]['level']<8){
			if($this->selectDomainCount($vo[0]['id']) == C('DOMAIN_LIMIT')){
				$this->ajaxReturn(0,'记录已经上限，最多50条解析记录',0);
			}
		}
		
		$entity = $domain->query("select * from domain d left join client_domain cd on d.id=cd.domain_id where host='".$_POST['host']."' and type='".$_POST['type']."' and view=".$_POST['view']." and val='".$_POST['val']."' and mx=".$_POST['mx']." and cd.zone_id=".$vo[0]['id']." and cd.client_id=".$_SESSION['id']);
		
		if(!empty($entity)){
			$this->ajaxReturn(0,'请不要添加相同的解析记录',0);
		}
		/*$where = array('host'=>$_POST['host'],'type'=>$_POST['type'],'view'=>$_POST['view'],'val'=>$_POST['val'],'mx'=>$_POST['mx']);
		$entity = $domain->where($where)->select();
		if(!empty($entity)){
			$this->ajaxReturn(0,'请不要添加相同的解析记录。',0);
		}*/
		
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
			$val = array("name"=>strtolower($data['host']).".".$_POST['zone'], "main"=>$_POST['zone'], "rid"=>(int)$is_ok, $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			//别名添加记录
			$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.domain='".$_POST['zone']."'");
			if(!empty($bmlist)){
				foreach($bmlist as $val){
					$val = array("name"=>strtolower($data['host']).".".$val['name'], "main"=>$val['name'], "rid"=>(int)$is_ok + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
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
				$data1['client_id'] = $_SESSION['id'];
				$data1['zone_id'] = $vo[0]['id'];
				$data1['domain_id'] = $is_ok;
				$client_domain->add($data1);
				
				$v = M('view');
				$view = $v->where('id='.$_POST['view'])->find();
				
				$history = file_get_contents(C('HISTORY_URL')."?opt=add&target=".$_POST['zone']."&class=domain&content=".$_POST['zone']."%20在".$view['name']."线路%20添加了记录值".$_POST['val']."的".$_POST['type']."记录");				
				$history_list = json_decode($history,true);
				$this->updateZoneTime($vo[0]['id']);
				
				$this->ajaxReturn($is_ok,'解析记录添加成功',1);
			}
		}else{
			$this->ajaxReturn(0,'添加失败，联系管理员',0);
		}
	}
	//绑定解析线路
	public function bindRoute(){
		if(!empty($_POST['id'])){
			$view = M('view');
			$vo = $view->where('id='.$_POST['id'])->select();
			$this->ajaxReturn($vo,'success',1);
		}
	}
	//修改解析记录
	public function updateDomain(){		
		$domain = M('domain');
		$zone = M('zone');
		//时间间隔
		$up_time = $domain->where('id='.$_POST['id'])->getField('up_time');
		if(strtotime(date("Y-m-d H:i:s"))-strtotime($up_time)<10){
			$this->ajaxReturn('请不要频繁修改解析记录。','error',0);
		}
		
		
		$vo = $zone->where('domain="'.$_POST['zone'].'"')->select();
		$entity = $domain->query("select * from domain d left join client_domain cd on d.id=cd.domain_id where host='".$_POST['host']."' and type='".$_POST['type']."' and view=".$_POST['view']." and val='".$_POST['val']."' and mx=".$_POST['mx']." and cd.client_id=".$_SESSION['id']." and cd.zone_id=".$vo[0]['id']." and d.id not in (".$_POST['id'].") ");
		
		if(!empty($entity)){
			$this->ajaxReturn('请不要修改相同的解析记录。','error',0);
		}
		
		$data['host'] = $_POST['host'];
		$data['type'] = $_POST['type'];
		$data['view'] = $_POST['view'];
		$data['val'] = $_POST['val'];
		$data['mx'] = $_POST['mx'];
		$data['ttl'] = $_POST['ttl'];
		$data['up_time'] = date("Y-m-d H:i:s");
		
		$v = M('view');
		$view = $v->where('id='.$_POST['view'])->find();
		
		$brfore_update_entity = $domain->where('id='.$_POST['id'])->find();
		$old_view = $v->where('id='.$brfore_update_entity['view'])->find();
		
		$str = $_POST['zone']."%20将主机记录:".$brfore_update_entity['host'].",线路:".$old_view['name'].",记录值:".$brfore_update_entity['val']."的".$brfore_update_entity['type']."记录%20修改成%20主机记录:".$_POST['host'].",线路:".$view['name'].",记录值:".$_POST['val']."的".$_POST['type']."记录";
		
		//$datae = json_encode(array("data" => $data['val']));
		//$data_url = http_build_query ($datae);
		//print_r($data_url);exit; 
		
		$val = array("name"=>strtolower($data['host']).".".$_POST['zone'], "main"=>$_POST['zone'], "rid"=>(int)$_POST['id'], $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
		$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
		$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
		$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
		$rslt = json_decode($ret["content"],true);
		//别名修改记录
		$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.domain='".$_POST['zone']."'");
		if(!empty($bmlist)){
			foreach($bmlist as $value){
				$val = array("name"=>strtolower($data['host']).".".$val['name'], "main"=>$val['name'], "rid"=>(int)$_POST['id'] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>rawurlencode(strtolower($data['val'])), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view']);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn($rslt["error"],'error',0);
				}
			}
		}
		if($rslt["ret"] == 0){
			$is_ok = $domain->where('id='.$_POST['id'])->save($data);
			if($is_ok===false){
				$this->ajaxReturn($data,'error',0);
			}
			
			$history = file_get_contents(C('HISTORY_URL')."?opt=add&target=".$_POST['zone']."&class=domain&content=".$str);				
			$history_list = json_decode($history,true);
			$this->updateZoneTime($vo[0]['id']);
			
			$this->ajaxReturn($is_ok,'success',1);
		}else{
			$this->ajaxReturn($rslt["error"],'error',0);
		}
	}
	//删除解析记录
	public function deleteDomain(){
		if(!empty($_POST['id'])){
			$client_domain = M('client_domain');
			$domain = M('domain');
			$zone = M('zone');
			$arr = explode(',',trim($_POST['id']));
			//$this->ajaxReturn(0,$arr,0);
			$vo = $zone->where('domain="'.$_POST['zone'].'"')->select();
			$retnum = 0; $error = "";
			for($i=0;$i<count($arr);$i++){
				//域名删除记录
				$data = $domain->where('id='.$arr[$i])->find();
				$val = array("rid"=>(int)$arr[$i], $data['type']=>$data['val']);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				//别名删除记录
				$bmlist = $zone->query("select * from zone_name zn left join zone z on z.id=zn.zone_id where z.domain='".$_POST['zone']."'");
				if(!empty($bmlist)){
					foreach($bmlist as $val){
						$data = $domain->where('id='.$arr[$i])->find();
						$val = array("rid"=>(int)$arr[$i] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>strtolower($data['val']));
						$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
						$param = array("type"=>"record", "opt"=>"del", "data"=>$val,"user"=>$user);
						$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
						$rslt = json_decode($ret["content"],true);
						if($rslt["ret"] != 0){
							$this->ajaxReturn($rslt["content"],"error",0);	
						}
					}
				}
				
				//$this->ajaxReturn(0,$ret["content"],0);
				if($rslt["ret"] == 0){
					$client_domain->where('domain_id='.$arr[$i].' and zone_id='.$vo[0]['id'])->delete();
					if($arr[$i]==1 || $arr[$i]==2 || $arr[$i]==3 || $arr[$i]==4){
						continue;	
					}
					$domain->where('id='.$arr[$i])->delete();
				}else{
					$retnum += 1;
					$error = $ret["content"];//$rslt["error"];
				}
				$history = file_get_contents(C('HISTORY_URL')."?opt=add&target=".$_POST['zone']."&class=domain&content=".$_POST['zone']."%20删除了记录值".$data['val']."的".$data['type']."记录");				
				$history_list = json_decode($history,true);				
				$this->updateZoneTime($vo[0]['id']);
			}
			if($retnum > 0){
				$this->ajaxReturn(0,$error,0);
			}else{
				
				$this->ajaxReturn(1,'success',1);
			}
		}
	}	
	//绑定域名搜索数据
	public function bindSearch(){
		$zone = M('zone');
		$vo = $zone->where('client_id='.$_SESSION['id'])->select();
		$this->ajaxReturn($vo,'success',1);
	}
	//检查别名重复
	public function checkBiem(){
		if(!empty($_POST['name'])){
			$zone_name = M('zone_name');
			$vo = $zone_name->where("name='".$_POST['name']."'")->select();
			if($vo){
				$this->ajaxReturn($vo,'error',1);
		   	}else{
				$this->ajaxReturn($vo,'success',1);
		   	}
		}		
	}
	//添加别名
	public function addBiem(){
		if(!empty($_POST['name']) && !empty($_POST['zone'])){
			
			$zone_name = M('zone_name');
			$arr = explode(',',trim($_POST['name']));
			$vo = M('zone')->where('domain="'.$_POST['zone'].'"')->find();
			
			for($i=0;$i<count($arr);$i++){
				$val = array("name"=>$arr[$i]);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"domain", "opt"=>"add", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn(0,'添加失败，联系管理员',0);
				}
				//别名下面添加记录
				$domain = M('client_doamin');
				$reclist = $domain->query('select * from client_domain cd left join domain d on cd.domain_id=d.id where cd.zone_id='.$vo['id']);
				if(!empty($reclist)){					
					foreach($reclist as $value){
						if($value['type']=='NS'){	
							$ds = array("name"=>$value['host'].".".$value['domain'], "main"=>$arr[$i], "rid"=>(int)$value['id'] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), "domain_ns"=>$value['val'], "level"=>$value['mx'], "ttl"=>(int)($value['ttl'])*60, "viewid"=>$value['view']);
							$use = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
							$par = array("type"=>"record", "opt"=>"add", "data"=>$ds, "user"=>$use);
							$rt = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $par);
							$rs = json_decode($rt["content"], true);				
							if($rs["ret"] != 0){ 
								$this->ajaxReturn(0,'添加失败，联系管理员',0);
							}
						}else{
							$val = array("name"=>$value['host'].".".$arr[$i], "main"=>$arr[$i], "rid"=>(int)$value['id'] + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $value['type']=>rawurlencode($value['val']), "level"=>$value['mx'], "ttl"=>(int)($value['ttl'])*60, "viewid"=>$value['view']);
							$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
							$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
							$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
							$rslt = json_decode($ret["content"],true);
							if($rslt["ret"] != 0){
								$this->ajaxReturn(0,'添加失败，联系管理员',0);
							}
						}
						
						
					}
				}
			}
			for($i=0;$i<count($arr);$i++){
				if(!empty($arr[$i])){
					$data['zone_id'] = $vo['id'];
					$data['name'] = $arr[$i];
					$is_ok = $zone_name->add($data);
					if($is_ok == false){
						$this->ajaxReturn(0,'添加失败，联系管理员',0);
					}
				}
			}
			
			$this->ajaxReturn($data,'success',1);
		}	
	}
	//删除别名
	public function deleteBiem(){
		if(!empty($_POST['name'])){
			
			$val = array("name"=>$_POST['name']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"domain", "opt"=>"del", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] != 0){
				$this->ajaxReturn(0,'删除失败，联系管理员',0);
			}
			
			$vo = M('zone_name')->where('name="'.$_POST['name'].'"')->delete();
			if($vo==false){
				$this->ajaxReturn(0,'删除失败，联系管理员',0);
			}
			$this->ajaxReturn($vo,'success',1);
		}
	}
	//批量停用域名
	public function stopZone(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$arr = explode(',',trim($_POST['domain']));
			
			for($i=0;$i<count($arr);$i++){
				if(!empty($arr[$i])){
					$data = $zone->where('domain="'.$arr[$i].'"')->find();
					$val = array("name"=>$data['domain'],"enable"=>0);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"domain", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}					
				}
			}
			//查看是否有别名
			for($i=0;$i<count($arr);$i++){
				$this->zone_biem_update($arr[$i],0);
			}
			
			$is_ok = $zone->where(array('domain'=>array('in',$_POST['domain'])))->setField('is_on',0);
			if($is_ok===false){
				$this->ajaxReturn(0,'域名停用失败，请联系管理员',0);	
			}
			
			$this->ajaxReturn($zone,'success',1);
		}
	}
	//批量启用域名
	public function startZone(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$arr = explode(',',trim($_POST['domain']));
			$str;
			for($i=0;$i<count($arr);$i++){
				if(!empty($arr[$i])){
					$data = $zone->where('domain="'.$arr[$i].'"')->find();
					$val = array("name"=>$data['domain'],"enable"=>1);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"domain", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}				
					$str .= $arr[$i] . ',';
				}
			}
			//查看是否有别名
			for($i=0;$i<count($arr);$i++){
				$this->zone_biem_update($arr[$i],1);
			}
			$is_ok = $zone->where(array('domain'=>array('in',$_POST['domain'])))->setField('is_on',1);
			if($is_ok===false){
				$this->ajaxReturn(0,'域名启用失败，请联系管理员',0);
			}
			
			$str = substr($str,0,(strlen($str)-1));
			$data = $zone->where(array('domain'=>array('in',$str)))->select();
			$this->ajaxReturn($data,'success',1);
		}
	}
	//域名的别名修改
	public function zone_biem_update($domain,$enable){
		$zone = M('zone');
		$data = $zone->where('domain="'.$domain.'"')->find();
		$zone_name = M('zone_name');
		$list = $zone_name->where('zone_id='.$data['id'])->find();
		if(!empty($list)){
			foreach($list as $val){
				$val = array("name"=>$val['name'],"enable"=>$enable);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"domain", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn($rslt["error"],'error',0);
				}
			}
		}
	}
	
	//查询是否被锁定
	public function selZoneLock(){
		if(!empty($_POST['zone'])){
			$vo = M('zone')->where("domain='".$_POST['zone']."'")->select();
			if($vo){
				$this->ajaxReturn($vo,'success',1);
			}else{
				$this->ajaxReturn(0,'查询锁定失败，请联系管理员',0);
			}
		}
	}
	//批量查询是否锁定
	public function batchSelZoneLock(){
		if(!empty($_POST['zone'])){
			$zone = M('zone');
			$data = $zone->where(array('domain'=>array('in',$_POST['zone'])))->select();
			if($data){
				for($i=0;$i<count($data);$i++){
					if($data[$i]['is_lock']==1){
						$this->ajaxReturn('['.$data[$i]['domain'].']域名被锁定，不能执行该操作','success',1);
					}
				}
				$this->ajaxReturn('success','success',1);
			}else{
				$this->ajaxReturn(0,'查询锁定失败，请联系管理员',0);
			}
		}
	}
	//添加分组
	public function addGroup(){
		if(!empty($_POST['name'])){
			$client_group = M('client_group');
			$vo = $client_group->query('select * from client_group where `group`="'+$_POST['name']+'" and client_id=%d ',$_SESSION['id']);
			if($vo){
				$this->ajaxReturn('同名分组已存在，请重新输入','error',0);
			}else{
				$data['client_id'] = $_SESSION['id'];
				$data['group'] = $_POST['name'];
				$is_ok = $client_group->add($data);
				if($is_ok){
					$this->ajaxReturn($data,'success',1);
				}else{
					$this->ajaxReturn('分组添加失败，请联系管理员','error',0);
				}
			}
		}
	}
	//检查分组名是否存在
	public function groupBy(){
		if(!empty($_POST['name'])){
			$client_group = M('client_group');
			$zone = M('zone');
			$vo = $client_group->query('select * from client_group where `group`="'.$_POST['name'].'" and client_id=%d ',$_SESSION['id']);
			if($vo){
				$zone->where(array('domain'=>array('in',$_POST['zone'])))->setField('group_id',$vo[0]['id']);
			}else{	
				$data['client_id'] = $_SESSION['id'];
				$data['group'] = $_POST['name'];
				$is_ok = $client_group->add($data);
				if($is_ok){										
					$zone->where(array('domain'=>array('in',$_POST['zone'])))->setField('group_id',$is_ok);
					$this->ajaxReturn($data,'success',1);
				}else{
					$this->ajaxReturn('分组添加失败，请联系管理员','error',0);
				}
			}
		}
	}
	//解析记录的别名修改
	public function domain_biem_update($id,$zone,$enable){
		$domain = M('domain');
		$data = $domain->where('id='.$id)->find();
		
		$zone = M('zone');
		$tem = $zone->where('domain="'.$zone.'"')->find();
		
		$zone_name = M('zone_name');
		$list = $zone_name->where('zone_id='.$tem['id'])->find();
		
		if(!empty($list)){
			foreach($list as $value){
				$val = array("name"=>$data['host'].".".$zone, "main"=>$zone, "rid"=>(int)$id + (int)str_pad(1,17,'0',STR_PAD_RIGHT), $data['type']=>rawurlencode($data['val']), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view'],"enable"=>$enable);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				if($rslt["ret"] != 0){
					$this->ajaxReturn($rslt["error"],'error',0);
				}	
			}
		}
	}
	//启用解析记录
	public function startDns(){
		if(!empty($_POST['id'])){
			$domain = M('domain');
			$arr = explode(",",$_POST['id']);
			for($i=0;$i<count($arr);$i++){
				$data = $domain->where('id='.$arr[$i])->find();
				$val = array("name"=>$data['host'].".".$_POST['zone'], "main"=>$_POST['zone'], "rid"=>$arr[$i], $data['type']=>rawurlencode($data['val']), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view'],"enable"=>1);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				
				if($rslt["ret"] != 0){
					$this->ajaxReturn($rslt["error"],'error',0);
				}				
				$history = file_get_contents(C('HISTORY_URL')."?opt=add&target=".$_POST['zone']."&class=domain&content=".$_POST['zone']."%20启用了记录值".$data['val']."的".$data['type']."记录");				
				$history_list = json_decode($history,true);
			}
			//是否有别名
			for($i=0;$i<count($arr);$i++){
				$this->domain_biem_update($arr[$i],$_POST['zone'],1);
			}
			
			$is_ok = $domain->where(array('id'=>array('in',$_POST['id'])))->setField('is_on',1);
			if($is_ok===false){
				$this->ajaxReturn(0,'error',0);
			}
			$this->ajaxReturn($is_ok,'success',1);
		}
	}
	//停用解析记录
	public function stopDns(){
		if(!empty($_POST['id'])){
			$domain = M('domain');
			$arr = explode(",",$_POST['id']);
			for($i=0;$i<count($arr);$i++){				
				$data = $domain->where('id='.$arr[$i])->find();
				$val = array("name"=>$data['host'].".".$_POST['zone'], "main"=>$_POST['zone'], "rid"=>(int)$arr[$i], $data['type']=>rawurlencode($data['val']), "level"=>$data['mx'], "ttl"=>(int)($data['ttl'])*60, "viewid"=>$data['view'],"enable"=>0);
				$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
				$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
				$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
				$rslt = json_decode($ret["content"],true);
				
				if($rslt["ret"] != 0){
					$this->ajaxReturn($rslt["error"],'error',0);
				}				
				$history = file_get_contents(C('HISTORY_URL')."?opt=add&target=".$_POST['zone']."&class=domain&content=".$_POST['zone']."%20停用了记录值".$data['val']."的".$data['type']."记录");				
				$history_list = json_decode($history,true);
			}
			//是否有别名
			for($i=0;$i<count($arr);$i++){
				$this->domain_biem_update($arr[$i],$_POST['zone'],0);
			}
			$is_ok = $domain->where(array('id'=>array('in',$_POST['id'])))->setField('is_on',0);
			if($is_ok===false){
				$this->ajaxReturn(0,'error',0);
			}
			$this->ajaxReturn($is_ok,'success',1);
		}
	}
	//绑定解析记录搜索框
	public function bindDomainsch(){
		if(!empty($_GET['zone'])){
			$zone = M('zone');
			$domain = M('domain');
			$vo = $zone->where('domain="'.$_GET['zone'].'"')->select();
			$dmList = $domain->query('select * from client_domain cd left join domain d on cd.domain_id=d.id where cd.zone_id=%d and cd.client_id=%d ',$vo[0]['id'],$_SESSION['id']);
			if($dmList){
				$this->ajaxReturn($dmList,'success',1);	
			}else{
				$this->ajaxReturn(0,'error',0);	
			}
		}
	}
	//域名锁定
	public function zoneLock(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$vo = $zone->where('domain="'.$_POST['domain'].'"')->select();
			if($vo[0]['is_lock']==0){
				$is_ok = $zone->where('domain="'.$_POST['domain'].'"')->setField('is_lock',1);
				if($is_ok){
					$this->ajaxReturn(1,'success',1);	
				}else{
					$this->ajaxReturn('域名锁定失败，请联系管理员','error',0);	
				}
			}else{
				$is_ok = $zone->where('domain="'.$_POST['domain'].'"')->setField('is_lock',0);
				if($is_ok){
					$this->ajaxReturn(0,'success',1);	
				}else{
					$this->ajaxReturn('域名解锁失败，请联系管理员','error',0);	
				}
			}
		}
	}
	//停启用域名
	public function startStopZone(){
		if(!empty($_POST['domain'])){
			$zone = M('zone');
			$vo = $zone->where('domain="'.$_POST['domain'].'"')->select();
			if($vo[0]['is_on']==0){
				$is_ok = $zone->where('domain="'.$_POST['domain'].'"')->setField('is_on',1);
				if($is_ok){
					$val = array("name"=>$_POST['domain'],"enable"=>1);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"domain", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}else{
						$this->ajaxReturn(1,'success',1);		
					}
				}else{
					$this->ajaxReturn('域名启用失败，请联系管理员','error',0);	
				}
			}else{
				$is_ok = $zone->where('domain="'.$_POST['domain'].'"')->setField('is_on',0);
				if($is_ok){
					$val = array("name"=>$_POST['domain'],"enable"=>0);
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"domain", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}else{
						$this->ajaxReturn(0,'success',1);	
					}				
				}else{
					$this->ajaxReturn('域名停用失败，请联系管理员','error',0);
				}
			}
		}
	}
	//添加管理权限
	public function doAddUser(){
		if(!empty($_POST['user'])){
			$client = M('client');
			$client_domain = M('client_domain');
			$user = $client->where('mail="'.$_POST['user'].'"')->find();
			if($user){
				if($_POST['type']=='全部记录'){
					$client_domain->where('client_id='.$user['id'].' and zone_id='.$_POST['zoneID'])->delete();					
					$domainList = $client_domain->where('client_id='.$_SESSION['id'].' and zone_id='.$_POST['zoneID'])->select();
					for($i=0;$i<count($domainList);$i++){
						$data[$i]['client_id'] = $user['id'];
						$data[$i]['zone_id'] = $_POST['zoneID'];
						$data[$i]['domain_id'] = $domainList[$i]['domain_id'];
						$is_ok = $client_domain->add($data[$i]);
						if($is_ok==false){							
							$this->ajaxReturn('添加管理权限失败，请联系管理员','error',0);	
						}
					}
					$this->ajaxReturn($data,'success',1);
				}else{
					$tem = $client_domain->where('client_id='.$user['id'].' and zone_id='.$_POST['zoneID'].' and domain_id='.$_POST['domainID'])->select();
					if($tem){
						$this->ajaxReturn('该用户已经拥有管理权限','error',0);	
					}else{
						$data['client_id'] = $user['id'];
						$data['zone_id'] = $_POST['zoneID'];
						$data['domain_id'] = $_POST['domainID'];
						$is_ok = $client_domain->add($data);
						if($is_ok==false){							
							$this->ajaxReturn('添加管理权限失败，请联系管理员','error',0);	
						}else{
							$this->ajaxReturn($data,'success',1);
						}
					}
				}
			}else{
				$this->ajaxReturn('用户名不存在，请重新输入','error',0);
			}
		}
	}
	//删除管理账户
	public function deleteUser(){
		if(!empty($_POST['zid']) && !empty($_POST['cid'])){
			$client_domain = M('client_domain');
			$is_ok = $client_domain->where('client_id='.$_POST['cid'].' and zone_id='.$_POST['zid'])->delete();
			if($is_ok){
				$this->ajaxReturn('删除管理权限用户成功','success',1);
			}else{
				$this->ajaxReturn('删除管理账户失败，请联系管理员','error',0);
			}
		}
	}
	//域名过户
	public function transfer(){
		if(!empty($_POST['email'])){
			$client = M('client');
			$zone = M('zone');
			$client_domain = M('client_domain');
			$client_group = M('client_group');
			$vo = $client->where('mail="'.$_POST['email'].'"')->find();
			if($vo){
				if($vo['id']==$_SESSION['id']){
					$this->ajaxReturn('域名过户不能填入自己的账户，请重新输入','error',0);
				}else{
					//修改client_id有关数据
					$is_ok = $client_domain->where('client_id='.$_SESSION['id'])->setField('client_id',$vo['id']);
					$is_ok = $client_group->where('client_id='.$_SESSION['id'])->setField('client_id',$vo['id']);
					$is_ok = $zone->where('client_id='.$_SESSION['id'])->setField('client_id',$vo['id']);
					if($is_ok===false){
						$this->ajaxReturn('域名过户失败，请联系管理员','error',0);
					}else{
						$this->ajaxReturn(1,'success',1);
					}
				}
			}else{
				$this->ajaxReturn('DNSPro账户不存在，请重新输入','error',0);
			}
		}
	}
	//修改默认ttl值
	public function changeTTL(){
		if(!empty($_POST['id']) && !empty($_POST['ttl'])){
			$zone = M('zone');
			$is_ok = $zone->where('id='.$_POST['id'])->setField('default_ttl',$_POST['ttl']);
			if($is_ok){
				$this->ajaxReturn(1,'success',1);
			}else{
				$this->ajaxReturn('域名TTL默认值修改失败，请联系管理员','error',0);
			}
		}	
	}
	//添加自定义线路
	public function addRoute(){
		if(!empty($_POST['name']) && !empty($_POST['ips'])){
			
			$route = M('route');
			$route_mask = M('route_mask');
			$vo = $route->where('name="'.$_POST['name'].'"')->select();
			if($vo){
				$this->ajaxReturn('自定义线路的名称重复，请重新输入','error',0);
			}else{
				$data['zone_id'] = $_POST['id'];
				$data['name'] = $_POST['name'];
				$is_ok = $route->add($data);
				$arr = explode(';',trim($_POST['ips']));
				for($i=0;$i<count($arr);$i++){
					if(!empty($arr[$i])){
							$tem = explode('-',trim($arr[$i]));
							//ip转换整型
							$data[$i]['route_id'] = $is_ok;
							$data[$i]['mask_start'] = ip2long($tem[0]);
							$data[$i]['mask_end'] = ip2long($tem[1]);
							$ok = $route_mask->add($data[$i]);
							if($ok==false){							
								$this->ajaxReturn('添加自定义线路错误，请联系管理员','error',0);
							}
						}
					}				
				$this->ajaxReturn($data,'success',1);
			}			
		}
	}
	//删除自定义线路
	public function deleteRoute(){
		if(!empty($_POST['name'])){
			$route = M('route');
			$route_mask = M('route_mask');
			$vo = $route->where('name="'.$_POST['name'].'"')->find();
			$is_ok = $route->where('id='.$vo['id'])->delete();
			$route_mask->where('route_id='.$vo['id'])->delete();
			if($is_ok==false){
				$this->ajaxReturn('删除自定义线路失败，请联系管理员','error',0);
			}else{
				$this->ajaxReturn('自定义线路删除成功','success',1);
			}
		}
	}
	//修改自定义线路
	public function updateRoute(){
		if(!empty($_POST['name']) && !empty($_POST['ips'])){
			$route = M('route');
			$route_mask = M('route_mask');
			$vo = $route->where('name="'.$_POST['name'].'"')->find();
			$route_mask->where('route_id='.$vo['id'])->delete();
			
			$arr = explode(';',trim($_POST['ips']));
			for($i=0;$i<count($arr);$i++){
				if(!empty($arr[$i])){
					$tem = explode('-',trim($arr[$i]));
					//ip转换整型
					$data[$i]['route_id'] = $vo['id'];
					$data[$i]['mask_start'] = ip2long($tem[0]);
					$data[$i]['mask_end'] = ip2long($tem[1]);
					$ok = $route_mask->add($data[$i]);
					if($ok==false){							
						$this->ajaxReturn('修改自定义线路错误，请联系管理员','error',0);
					}
				}
			}
			$this->ajaxReturn($data,'success',1);
		}
	}
	
	//模糊查询记录
	public function selectDomain(){
//		if(!empty($_POST['val'])){	
			$zone = M('zone');
			$domain = M('domain');
			$vo = $zone->where('domain="'.$_POST['zone'].'"')->find();
			//print($zone->getLastSql());exit;
			
			$dmList = $domain->where(array("cd.zone_id"=>$vo['id'],"cd.client_id"=>$_SESSION['id'],"d.host"=>array("like","%".$_POST['val']."%")))->query("select * from client_domain cd left join domain d on cd.domain_id=d.id %WHERE%",true);
						
			if($dmList){
				$this->ajaxReturn($dmList,'success',1);	
			}else{
				$this->ajaxReturn(0,'error',0);	
			}
//		}
	}
	
	//模糊查询域名
	public function selectZone(){
		$zone = M('zone');
		$vo = $zone->where(array('client_id'=>$_SESSION['id'],'domain'=>array('like',"%".$_POST['val']."%")))->select();
		
		$name = M('zone_name');
		$client_domain = M('client_domain');
		foreach($vo as $n => $val){
			$ret = $name->where('zone_id='.$val['id'])->select();
			$vo[$n]["count"] = $client_domain->where('zone_id='.$val['id'])->count();
			
			if(empty($ret)){
				$vo[$n]["zone_name"] = 	0;
			}else{
				$vo[$n]["zone_name"] = 	1;
			}
		}
		$this->ajaxReturn($vo,'success',1);		
	}
	 
	
}

