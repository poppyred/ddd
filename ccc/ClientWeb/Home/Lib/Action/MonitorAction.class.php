<?php
// 本类由系统自动生成，仅供测试用途
class MonitorAction extends BaseAction {

	public function index(){
		if(empty($_GET['type'])){
			$mlist = $this->returnMlist();
			$this->assign('mlist',$mlist);
			$this->display();	
		}else{
			switch($_GET['type']){
				case 'all' :
					$mlist = $this->returnMlist();
					$this->assign('type',$_GET['type']);
					$this->assign('mlist',$mlist);
					$this->display();	
				break;
				case 'success' :
					$mlist = $this->returnMlist();
					$list;
					foreach($mlist as $key => $val){
						if($val['status']=="<label style='color:green'>正常</label>"){
							$list[$key] = $val;
						}
					}
					$this->assign('type',$_GET['type']);
					$this->assign('mlist',$list);
					$this->display();	
				break;
				case 'error' :
					$mlist = $this->returnMlist();
					$list;
					foreach($mlist as $key => $val){
						if($val['status']=="<label style='color:red'>异常</label>"){
							$list[$key] = $val;
						}
					}
					$this->assign('type',$_GET['type']);
					$this->assign('mlist',$list);
					$this->display();	
				break;
			}
		}		
	}
	
	public function detail(){
		if(!empty($_GET['d'])){
			$monitor = M('monitor');
			$iplist = $monitor->query("select * from monitor m left join monitor_info mi on m.id=mi.monitor_id where m.domain='".$_GET['d']."' and mi.type=0");
			$urllist = $monitor->query("select * from monitor m left join monitor_info mi on m.id=mi.monitor_id where m.domain='".$_GET['d']."' and mi.type=1");
			
			$this->assign('iplist', $iplist);
			$this->assign('urllist', $urllist);
			$this->assign('domain', $_GET['d']);
			$this->display();
		}
	}
	
	public function add(){
		$zone = M('zone');
		$zlist = $zone->where('client_id='.$_SESSION['id'].' and level>0')->select();
		
		$domain = M('domain');
		//判断是否存在monitor表
		$monitor = M('monitor');
		$mlist = $monitor->where("client_id=".$_SESSION['id']." and domain like '%". $zlist[0]['domain'] ."%'")->select();
		
		if(!empty($mlist)){
			foreach($mlist as $m){
				$str .= "'" . substr(str_replace($zlist[0]['domain'],'',$m['domain']),0,-1) . "'," ;
			}
			$dlist = $domain->query("select distinct(d.host),d.type from `zone` z left join client_domain cd on z.id = cd.zone_id left join domain d on d.id = cd.domain_id where z.client_id=%d and z.level>0 and (d.type='A' or d.type='CNAME') and z.domain='".$zlist[0]['domain']."' and d.host not in (".substr($str,0,-1).")",$_SESSION['id']);
			foreach($dlist as $key => $val){
				$dlist[$key]['val'] = $val['host'].'.'.$zlist[0]['domain'];
				$dlist[$key]['type'] = $val['type'];
			}
		}else{
			$dlist = $domain->query("select distinct(d.host),d.type from `zone` z left join client_domain cd on z.id = cd.zone_id left join domain d on d.id = cd.domain_id where z.client_id=%d and z.level>0 and (d.type='A' or d.type='CNAME') and z.domain='".$zlist[0]['domain']."'",$_SESSION['id']);
			
			foreach($dlist as $key => $val){
				$dlist[$key]['val'] = $val['host'].'.'.$zlist[0]['domain'];
				$dlist[$key]['type'] = $val['type'];
			}
		}
		$this->assign('dlist', $dlist);
		$this->assign('zlist', $zlist);		
		$this->display();	
	}
	//绑定下拉框的子域名
	public function bindsel(){
		if(!empty($_POST['val'])){
			$domain = M('domain');
			//判断是否存在monitor表
			$monitor = M('monitor');
			$mlist = $monitor->query("select domain from `monitor` where client_id=".$_SESSION['id']." and domain like '%". $_POST['val'] ."%'");
			
			if(!empty($mlist)){
				foreach($mlist as $m){
					$str .=  "'" . substr(str_replace($_POST['val'],'',$m['domain']),0,-1) . "'," ;
				}
				$dlist = $domain->query("select distinct(d.host),d.type from `zone` z left join client_domain cd on z.id = cd.zone_id left join domain d on d.id = cd.domain_id where z.client_id=%d and z.level>0 and (d.type='A' or d.type='CNAME') and z.domain='".$_POST['val']."' and d.host not in (".substr($str,0,-1).")",$_SESSION['id']);
				foreach($dlist as $key => $val){
					$dlist[$key]['val'] = $val['host'].'.'.$_POST['val'];
					$dlist[$key]['type'] = $val['type'];	
				}
			}else{
				$dlist = $domain->query("select distinct(d.host),d.type from `zone` z left join client_domain cd on z.id = cd.zone_id left join domain d on d.id = cd.domain_id where z.client_id=%d and z.level>0 and (d.type='A' or d.type='CNAME') and z.domain='".$_POST['val']."' ",$_SESSION['id']);
				foreach($dlist as $key => $val){
					$dlist[$key]['val'] = $val['host'].'.'.$_POST['val'];
					$dlist[$key]['type'] = $val['type'];					
				}
			}
			$this->ajaxReturn($dlist,'success',1);
		}
	}
	//添加监控
	public function addMonitor(){
		if($_POST['type']=='ip'){
			//ip监控
			$domain = M('domain');
			//添加 monitor
			$monitor = M('monitor');
			$ret = $monitor->where("domain='".$_POST['val']."'")->find();
			$client = M('client');
			$c = $client->where("mail='".$_SESSION['user']."'")->find();
			
			if(empty($ret)){
				$data['client_id'] = $_SESSION['id'];
				$data['domain'] = $_POST['val'];
				$data['mail'] = $_SESSION['user'];
				$data['phone'] = $c['link_phone'];
				$is_ok = $monitor->add($data);
				if($is_ok===false){
					$this->ajaxReturn('添加域名监控失败，请联系管理员。','error',0);
				}
				$monitor_info = M('monitor_info');
				//替换主域名为空
				$str = str_replace($_POST['domain'],'',$_POST['val']);			
				$dlist = $domain->query("select d.val from `zone` z left join client_domain cd on z.id = cd.zone_id left join domain d on d.id = cd.domain_id where z.client_id=%d and z.level>0 and (d.type='A' or d.type='CNAME') and z.domain='".$_POST['domain']."' and d.host='".substr($str,0,-1)."'",$_SESSION['id']);
				if(!empty($dlist)){
					foreach($dlist as $d){
						$data['client_id'] = $_SESSION['id'];
						$data['monitor_id'] = $is_ok;
						$data['val'] = $d['val'];
						if($_POST['recordType']=='A'){
							$data['type'] = 0;
						}else{
							$data['type'] = 1;
						}					
						$is_ok2 = $monitor_info->add($data);
						if($is_ok2===false){
							$this->ajaxReturn('添加域名监控失败，请联系管理员。','error',0);
						}else{
							$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=add&val=["'. $d['val'] .'"]');
							$result = json_decode($data,true);
							if($result['ret']==0){
								$this->ajaxReturn(1,'success',1);
							}else{
								$this->ajaxReturn($result['descmap'],'error',0);
							}
						}
					}
				}
			}
		}else{
			//url监控
			$monitor = M('monitor');
			$monitor_info = M('monitor_info');
			$mlist = $monitor->where('client_id='.$_SESSION['id'])->select();
			$ok = false;
			$str = str_replace("http://","",$_POST['domain']);
			$str = str_replace(strstr($str,'/'),"",$str);
			//截取后的url查看是否在monitor表
			foreach($mlist as $m){
				if($m['domain']==$str){
					$ok = true;	
				}
			}
			if($ok){
				$m = $monitor->where("client_id=".$_SESSION['id']." and domain='".$str."'")->find();
				//存在monitor表,直接添加monitor_info记录
				$data['client_id'] = $_SESSION['id'];
				$data['monitor_id'] = $m['id'];
				$data['type'] = 1;
				$data['val'] = $_POST['domain'];//str_replace("http://","",);
				$is_ok = $monitor_info->add($data);
				if($is_ok2===false){
					$this->ajaxReturn('添加域名监控失败，请联系管理员。','error',0);
				}else{
					$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=add&val=["'. $_POST['domain'] .'"]');
					$result = json_decode($data,true);
					if($result['ret']==0){
						$this->ajaxReturn(1,'success',1);	
					}else{
						$this->ajaxReturn($result['descmap'],'error',0);
					}
				}
			}else{
				//不存在monitor就添加				
				$client = M('client');
				$c = $client->where("mail='".$_SESSION['user']."'")->find();
				$data['client_id'] = $_SESSION['id'];
				$data['domain'] = $str;
				$data['mail'] = $_SESSION['user'];
				$data['phone'] = $c['link_phone'];
				$is_ok = $monitor->add($data);
				if($is_ok===false){
					$this->ajaxReturn('添加域名监控失败，请联系管理员。','error',0);
				}
				$data2['client_id'] = $_SESSION['id'];
				$data2['monitor_id'] = $is_ok;
				$data2['type'] = 1;
				$data2['val'] = $_POST['domain'];//str_replace("http://","",);
				$is_ok2 = $monitor_info->add($data2);
				if($is_ok2===false){
					$this->ajaxReturn('添加域名监控失败，请联系管理员。','error',0);
				}else{
					$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=add&val=["'. $_POST['domain'] .'"]');
					$result = json_decode($data,true);
					if($result['ret']==0){
						$this->ajaxReturn(1,'success',1);	
					}else{
						$this->ajaxReturn($result['descmap'],'error',0);
					}
				}
			}
			
		}
	}
	public function deleteMonitorInfo(){
		if(!empty($_POST['domain']) && !empty($_POST['ip'])){
			$monitor = M('monitor');
			$mon = $monitor->where("domain='".$_POST['domain']."'")->find();
			$monitor_info = M('monitor_info');
			$is_ok = $monitor_info->where("val='".$_POST['val']."' and monitor_id=".$mon['id']." and client_id=".$_SESSION['id'])->delete();
			if($is_ok===false){
				$this->ajaxReturn('删除域名监控失败，请联系管理员。','error',0);
			}else{
				$this->ajaxReturn('','success',1);
			}
		}
	}
	//删除
	public function deleteMonitor(){
		if(!empty($_POST['id']) && !empty($_POST['type'])){
			$monitor = M('monitor');
			$monitor_info = M('monitor_info');
			if($_POST['type']=='one'){
				$milist = $monitor_info->where('monitor_id='.$_POST['id'])->select();
				$str;
				//删除监控接口
				foreach($milist as $val){
					$str .= '"'.$val['val'].'",';
				}
				$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=del&val=['.substr($str,0,-1).']');
				$result = json_decode($data,true);
				if($result['ret']!=0){
					$this->ajaxReturn($result['descmap'],'error',0);
				}
				//删除单个	
				$is_ok = $monitor_info->where('monitor_id='.$_POST['id'])->delete();
				if($is_ok===false){
					$this->ajaxReturn('删除域名监控失败，请联系管理员。','error',0);
				}
				$is_ok2 = $monitor->where('id='.$_POST['id'])->delete();
				if($is_ok2===false){
					$this->ajaxReturn('删除域名监控失败，请联系管理员。','error',0);
				}else{
					//从新查询监控记录				
					$mlist = $this->returnMlist();
					$this->ajaxReturn($mlist,'success',1);
				}
			}else{
				//删除多个
				$where['monitor_id'] = array('in',$_POST['id']);
				$milist = $monitor_info->where($where)->select();
				$str;
				//删除监控接口
				foreach($milist as $val){
					$str .= '"'.$val['val'].'",';
				}
				$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=del&val=['.substr($str,0,-1).']');
				$result = json_decode($data,true);
				if($result['ret']!=0){
					$this->ajaxReturn($result['descmap'],'error',0);
				}
				$is_ok = $monitor_info->where($where)->delete();
				if($is_ok===false){
					$this->ajaxReturn('删除域名监控失败，请联系管理员。','error',0);
				}				
				$where['id'] = array('in',$_POST['id']);
				$is_ok2 = $monitor->where($where)->delete();
				if($is_ok2===false){
					$this->ajaxReturn('删除域名监控失败，请联系管理员。','error',0);
				}else{
					//从新查询监控记录				
					$mlist = $this->returnMlist();
					$this->ajaxReturn($mlist,'success',1);
				}
			}
		}
	}
	//停止
	public function stopMonitor(){
		if(!empty($_POST['id'])){
			$monitor = M('monitor');
			$where['id'] = array('in',$_POST['id']);
			$monitor->where($where)->setField('is_on',1);
			if($is_ok2===false){
				$this->ajaxReturn('停用域名监控失败，请联系管理员。','error',0);
			}else{
				//从新查询监控记录				
				$mlist = $this->returnMlist();
				$this->ajaxReturn($mlist,'success',1);
			}			
		}
	}
	//启用
	public function startMonitor(){
		if(!empty($_POST['id'])){
			$monitor = M('monitor');
			$where['id'] = array('in',$_POST['id']);
			$monitor->where($where)->setField('is_on',0);
			if($is_ok2===false){
				$this->ajaxReturn('启用域名监控失败，请联系管理员。','error',0);
			}else{
				//从新查询监控记录				
				$mlist = $this->returnMlist();
				$this->ajaxReturn($mlist,'success',1);
			}
			
		}
	}
	//获取状态
	public function returnMlist(){		
		$monitor = M('monitor');
		$mlist = $monitor->where('client_id='.$_SESSION['id'])->select();
		foreach($mlist as $key =>$val){
			$mi = M('monitor_info');
			$milist = $mi->where('monitor_id='.$val['id'])->select();
			$str = "";
			foreach($milist as $value){
				$str .= '"' . $value['val'] . '",';
			}
			$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=status&val=['. substr($str,0,-1) .']');
			$result = json_decode($data,true);
			if($result['ret']==0){
				$mlist[$key]['status'] = "<label style='color:green'>正常</label>";
			}else{
				$mlist[$key]['status'] = "<label style='color:red'>异常</label>";
			}
			if(substr($val['domain'],0,2) == "@."){
				$mlist[$key]['domain'] = substr($val['domain'],2);
			}
		}
		return $mlist;
	}
	//获取图表
	public function getData(){
		$data = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=chart&val=["'. $_GET['val'] .'"]');
		$result = json_decode($data,true);
		
		$data2 = file_get_contents(C('NS_CHECH_URL').'/script/eflydns_monitor_domain.php?opt=status&val=["'. $_GET['val'] .'"]');
		$result2 = json_decode($data2,true);
		if($result['ret']==0){
			$result['descmap']['val'] = $_GET['val'];
			$result['descmap']['status'] = $result2['descmap'];
			$this->ajaxReturn($result['descmap'],'success',1);	
		}else{
			$this->ajaxReturn('','error',0);
		}
	}
}
