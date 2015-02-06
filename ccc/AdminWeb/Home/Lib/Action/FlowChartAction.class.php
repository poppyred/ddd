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
}
