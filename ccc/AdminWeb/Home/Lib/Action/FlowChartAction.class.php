<?php
// 本类由系统自动生成，仅供测试用途
class FlowChartAction extends BaseAction {
	public function index(){	
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			//$data = file_get_contents(C('API_URL')."/dns_chart_base.php?type=iplist");
			//$result = json_decode($data,true);
			$server = M('server_list');
			$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
			
			$view = M('view',Null,'DB_NEWS');
			$view_list = $view->select();
			
			if(!empty($_GET['ip'])){
				$this->assign('ip',$_GET['ip']);	
			}
			if(!empty($_GET['view'])){
				$this->assign('view',$_GET['view']);	
			}
			$this->assign('ipList',$result);
			$this->assign('viewList',$view_list);
			$this->display();
		}else{
			$url = C('API_URL')."/dns_chart_req.php?time=all";
			switch(true){
				case !empty($_POST['ip']) && !empty($_POST['view']):
					$url .= "&ip=" . $_POST['ip'] . "&view=" . $_POST['view'];
				break;
				case !empty($_POST['ip']) && empty($_POST['view']):
					$url .= "&ip=" . $_POST['ip'];
				break;
				case empty($_POST['ip']) && !empty($_POST['view']):
					$url .= "&view=" . $_POST['view'];
				break;
			}
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			
			if($result['ret']==0){
				$this->ajaxReturn($result,'success',1);
			}else{
				$this->ajaxReturn("获取数据失败！",'error',0);
			}
		}
	}	
	public function history(){			
		//$data = file_get_contents(C('API_URL')."/dns_chart_base.php?type=iplist");
		//$result = json_decode($data,true);
		$server = M('server_list');
		$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
		
		$view = M('view',Null,'DB_NEWS');
		$view_list = $view->select();
		
		$url = C('API_URL')."/dns_chart_req_history.php";
		if(empty($_GET['ip']) && empty($_GET['view']) && empty($_GET['time'])){
			$url .= "?timezone=today";
		}
		if(!empty($_GET['time'])){
			$url .= "?timezone=" . $_GET['time'];
			$this->assign('timezone', $_GET['time']);
		}
		if(!empty($_GET['ip'])){
			$url .= "&ip=" . $_GET['ip'];
			$this->assign('ip',$_GET['ip']);	
		}
		if(!empty($_GET['view'])){
			$url .= "&view=" . $_GET['view'];
			$this->assign('view',$_GET['view']);	
		}
		$data = file_get_contents($url);
		$list = json_decode($data,true);			
		if($list['ret']==0){
			foreach($list['descmap'] as $val){
				$time .= $val['time'] . ",";
				$num .= $val['req'] .",";
				$sum += $val['req'];
			}
		}
		$this->assign('sum', number_format($sum));
		$this->assign('time', trim($time,","));
		$this->assign('num', trim($num,","));
		$this->assign('ipList',$result);
		$this->assign('viewList',$view_list);
		$this->display();
	}	
	public function error(){
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			//$data = file_get_contents(C('API_URL')."/dns_chart_base.php?type=iplist");
			//$result = json_decode($data,true);
			
			$server = M('server_list');
			$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
			
			$view = M('view',Null,'DB_NEWS');
			$view_list = $view->select();
			
			if(!empty($_GET['ip'])){
				$this->assign('ip',$_GET['ip']);	
			}
			if(!empty($_GET['view'])){
				$this->assign('view',$_GET['view']);	
			}
			$this->assign('ipList',$result);
			$this->assign('viewList',$view_list);
			$this->display();
		}else{
			$url = C('API_URL')."/dns_chart_error.php?time=all";
			switch(true){
				case !empty($_POST['ip']) && !empty($_POST['view']):
					$url .= "&ip=" . $_POST['ip'] . "&view=" . $_POST['view'];
				break;
				case !empty($_POST['ip']) && empty($_POST['view']):
					$url .= "&ip=" . $_POST['ip'];
				break;
				case empty($_POST['ip']) && !empty($_POST['view']):
					$url .= "&view=" . $_POST['view'];
				break;
			}
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			if($result['ret']==0){
				$this->ajaxReturn($result,'success',1);
			}else{
				$this->ajaxReturn("获取数据失败！",'error',0);
			}
		}
	}
	public function error_history(){			
		//$data = file_get_contents(C('API_URL')."/dns_chart_base.php?type=iplist");
		//$result = json_decode($data,true);
		
		$server = M('server_list');
		$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
		
		$view = M('view',Null,'DB_NEWS');
		$view_list = $view->select();
		
		$url = C('API_URL')."/dns_chart_error_history.php";
		if(empty($_GET['ip']) && empty($_GET['view']) && empty($_GET['time'])){
			$url .= "?timezone=today";
		}
		if(!empty($_GET['time'])){
			$url .= "?timezone=" . $_GET['time'];
			$this->assign('timezone', $_GET['time']);
		}
		if(!empty($_GET['ip'])){
			$url .= "&ip=" . $_GET['ip'];
			$this->assign('ip',$_GET['ip']);	
		}
		if(!empty($_GET['view'])){
			$url .= "&view=" . $_GET['view'];
			$this->assign('view',$_GET['view']);	
		}
		$data = file_get_contents($url);
		$list = json_decode($data,true);			
		if($list['ret']==0){
			foreach($list['descmap'] as $val){
				$time .= $val['time'] . ",";
				$num .= $val['req'] .",";
				$sum += $val['req'];
			}
		}
		$this->assign('sum', number_format($sum));
		$this->assign('time', trim($time,","));
		$this->assign('num', trim($num,","));
		$this->assign('ipList',$result);
		$this->assign('viewList',$view_list);
		$this->display();
	}	
	//实时占比图
	public function proportion(){		
		$url = C('API_URL')."/dns_chart_viewper.php";
		if(!empty($_GET['ip'])){
			$url .= "?ip=" . $_GET['ip'];
			$this->assign('ip',$_GET['ip']);	
		}
		
		$data = file_get_contents($url);
		$list = json_decode($data,true);		
		if($list['ret'] != 0){
			$this->ajaxReturn("获取数据失败！",'error',0);
		}
		$server = M('server_list');
		$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
		
		$view = M('view',Null,'DB_NEWS');		
		$value = "[";
		foreach($list['descmap'] as $key => $val){
			$tem = $view->where('id='.$key)->find();
			$value .= "['".$tem['name']."',".$val."],";
		}
		$value = substr($value,0,-1) . "]";
		
		$this->assign('ipList',$result);	
		$this->assign('value',$value);
		$this->display();
		
	}
	//历史占比图
	public function history_proportion(){		
		$url = C('API_URL')."/dns_chart_viewper_history.php";
		switch(true){
			case empty($_GET['ip']) && empty($_GET['time']):
				$url .= "?timezone=today";
			break;
			case !empty($_GET['ip']) && !empty($_GET['time']):
				$url .= "?ip=" . $_GET['ip'] . "&timezone=" . $_GET['time'];
				$this->assign('ip',$_GET['ip']);	
				$this->assign('timezone',$_GET['time']);	
			break;
			case !empty($_GET['ip']) && empty($_GET['time']):
				$url .= "?ip=" . $_GET['ip'];
				$this->assign('ip',$_GET['ip']);	
			break;
			case empty($_GET['ip']) && !empty($_GET['time']):
				$url .= "?timezone=" . $_GET['time'];
				$this->assign('timezone',$_GET['time']);	
			break;
		}
		$data = file_get_contents($url);
		$list = json_decode($data,true);		
		if($list['ret'] != 0){
			$this->ajaxReturn("获取数据失败！",'error',0);
		}
		$server = M('server_list');
		$result = $server->distinct(true)->field('ip')->where("type='proxy'")->getField('ip',true);
		
		$view = M('view',Null,'DB_NEWS');		
		$value = "[";
		foreach($list['descmap'] as $key => $val){
			$tem = $view->where('id='.$key)->find();
			$value .= "['".$tem['name']."',".$val."],";
		}
		$value = substr($value,0,-1) . "]";
		$this->assign('ipList',$result);	
		$this->assign('value',$value);
		$this->display();
		
	}
	public function domain_request(){	
		$this->show_top_30('eflydns_admin_chart_domain.php');
	}
	//public function child_domain_request(){
	//	$this->show_top_30('eflydns_admin_chart_domain.php');
	//}
	public function visit_source(){
		$this->show_top_30('eflydns_admin_chart_ip.php');
	}
	public function regional_distribution(){
		$this->show_top_30('eflydns_admin_chart_area.php');
	}
	public function show_top_30($php){
		if(!empty($_GET['startTime'])){
			$startTime = $_GET['startTime'];
			$this->assign('startTime',$_GET['startTime']);
		}else{
			$startTime = date("Y-m-d",strtotime("-1 day"));	
			$this->assign('startTime',$startTime);
		}
		if(!empty($_GET['endTime'])){
			$endTime = $_GET['endTime'];
			$this->assign('endTime',$_GET['endTime']);
		}else{
			$endTime = date("Y-m-d",strtotime("-1 day"));
			$this->assign('endTime',$endTime);
		}
		
		if($php=="eflydns_admin_chart_domain.php"){			
			$url = "http://121.201.12.61/script/".$php."?begin=".$startTime."&end=".$endTime."&type=main";
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$list;
			if($result['ret'] != 1){
				foreach($result['descmap'] as $key => $val){
					$list[$key]['name'] = $key;
					$list[$key]['sum'] = $val;
					$zone = M('zone',Null,'DB_NEWS');
					$tem = $zone->query("select * from zone z left join client c on z.client_id=c.id where z.domain='".$key."'");
					$list[$key]['mail'] = $tem[0]['mail'];
					$list[$key]['pwd'] = $tem[0]['pwd'];
				}
			}
			$this->assign('list',$list);
			
			$url2 = "http://121.201.12.61/script/".$php."?begin=".$startTime."&end=".$endTime."&type=child";
			$data2 = file_get_contents($url2);
			$result2 = json_decode($data2,true);
			$list2; 
			if($result2['ret'] != 1){
				foreach($result2['descmap'] as $key => $val){
					$list2[$key]['name'] = $key;
					$list2[$key]['sum'] = $val;
				}
			}			
			$this->assign('list2',$list2);
		}else if($php=="eflydns_admin_chart_ip.php"){
			$url = "http://121.201.12.61/script/".$php."?begin=".$startTime."&end=".$endTime."&mod=top";
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$list; 
			if($result['ret'] != 1){
				foreach($result['descmap'] as $key => $val){
					$list[$key]['name'] = $key;
					$list[$key]['sum'] = $val;
				}
			}
			$this->assign('list',$list);
		}else{
			$url = "http://121.201.12.61/script/".$php."?begin=".$startTime."&end=".$endTime;
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$list; 
			if($result['ret'] != 1){
				foreach($result['descmap'] as $key => $val){
					$list[$key]['name'] = $key;
					$list[$key]['sum'] = $val;
				}
			}
			$this->assign('list',$list);
		}	
		$this->assign('nowTime',date("Y-m-d"));
		$this->display();	
	}
	public function show_ip_info(){
		$url = "http://121.201.12.61/script/eflydns_admin_chart_ip.php?begin=".$_GET['startTime']."&end=".$_GET['endTime']."&mod=info&ip=".$_GET['ip'];
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$list; 
			if($result['ret'] != 1){
				foreach($result['descmap'][1] as $key => $val){
					$list[$key]['name'] = $key;
					$list[$key]['sum'] = $val;
				}
				foreach($result['descmap'][0] as $key => $val){
					$time .= $key . "时,";
					$num .= $val .",";
					$sum += $val;
				}
			}
			$this->assign('sum', number_format($sum));
			$this->assign('time', trim($time,","));
			$this->assign('num', trim($num,","));
			$this->assign('list',$list);
		$this->display();	
	}
	
	public function saveAlarm(){
		if(!empty($_POST['type']) && isset($_POST['max']) && isset($_POST['min']) && !empty($_POST['mail']) && !empty($_POST['id'])){
			$device_qps = M('device_qps',Null,'DB_MONITOR');
			$data['type'] = $_POST['type'];
			$data['maximum'] = $_POST['max'];
			$data['minimum'] = $_POST['min'];
			$data['to'] = $_POST['mail'];
			
			$is_ok = $device_qps->where('id='.$_POST['id'])->save($data);
			if($is_ok===false){
				$this->ajaxReturn('修改监控报警失败，请联系管理员。','error',0);	
			}
			$this->ajaxReturn('修改监控报警成功','success',1);	
		}
	}
	
	public function getAlarmInfo(){
		if(!empty($_POST['type'])){
			$map['type'] = $_POST['type'];
			$device_qps = M('device_qps',Null,'DB_MONITOR');
			$result = $device_qps->where($map)->find();
			if(empty($result)){
				$this->ajaxReturn('监控信息获取失败，请联系管理员。','error',0);	
			}
			$this->ajaxReturn($result,'success',1);
		}
	}
	//导出Excel
	public function expExcel(){//导出Excel		
		if($_GET['type']=="request"){
			$xlsCell  = array(
				array('no','序号'),
				array('name','域名'),
				array('sum','次数')
			);
			$url = "http://121.201.12.61/script/eflydns_admin_chart_domain.php?begin=".$_GET['startTime']."&end=".$_GET['endTime']."&type=main";
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$no = 0;
			$xlsData;
			foreach($result['descmap'] as $key => $val){
				$xlsData[$no]['no'] = $no + 1;
				$xlsData[$no]['name'] = $key;
				$xlsData[$no]['sum'] = $val;
				$no ++;
			}
			D('Excel')->exportExcel("主域名请求TOP30排行榜",$xlsCell,$xlsData);
			
		}else if($_GET['type']=="child_request"){
			$xlsCell  = array(
				array('no','序号'),
				array('name','域名'),
				array('sum','次数')
			);
			$url = "http://121.201.12.61/script/eflydns_admin_chart_domain.php?begin=".$_GET['startTime']."&end=".$_GET['endTime']."&type=child";
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$no = 0;
			$xlsData;
			foreach($result['descmap'] as $key => $val){
				$xlsData[$no]['no'] = $no + 1;
				$xlsData[$no]['name'] = $key;
				$xlsData[$no]['sum'] = $val;
				$no ++;
			}
			D('Excel')->exportExcel("子域名请求TOP30排行榜",$xlsCell,$xlsData);
			
		}else if($_GET['type']=="source"){
			$xlsCell  = array(
				array('no','序号'),
				array('name','IP'),
				array('sum','次数')
			);
			$url = "http://121.201.12.61/script/eflydns_admin_chart_ip.php?begin=".$_GET['startTime']."&end=".$_GET['endTime']."&mod=top";
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$no = 0;
			$xlsData;
			foreach($result['descmap'] as $key => $val){
				$xlsData[$no]['no'] = $no + 1;
				$xlsData[$no]['name'] = $key;
				$xlsData[$no]['sum'] = $val;
				$no ++;
			}
			D('Excel')->exportExcel("访问来源TOP30排行榜",$xlsCell,$xlsData);
		}else if($_GET['type']=="distribution"){
			$xlsCell  = array(
				array('no','序号'),
				array('name','地域'),
				array('sum','次数')
			);
			$url = "http://121.201.12.61/script/eflydns_admin_chart_area.php?begin=".$_GET['startTime']."&end=".$_GET['endTime'];
			$data = file_get_contents($url);
			$result = json_decode($data,true);
			$no = 0;
			$xlsData;
			foreach($result['descmap'] as $key => $val){
				$xlsData[$no]['no'] = $no + 1;
				$xlsData[$no]['name'] = $key;
				$xlsData[$no]['sum'] = $val;
				$no ++;
			}
			D('Excel')->exportExcel("地域分布排行榜",$xlsCell,$xlsData);
		}	
	}
	public function report(){		
		if($_SERVER['REQUEST_METHOD' ] === 'GET'){
			$this->display();
		}else{
			if(!empty($_POST['mail'])){
				$url = "http://121.201.12.61/script/eflydns_admin_mail.php?target=".$_POST['mail'];
				$data = file_get_contents($url);
				$result = json_decode($data,true);
				if($result['ret']!=0){
					$this->ajaxReturn($result['error'],'error',0);
				}
				$this->ajaxReturn($result,'success',1);
			}
		}
	}
}
