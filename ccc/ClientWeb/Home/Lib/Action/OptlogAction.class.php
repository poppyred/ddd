<?php
class OptlogAction extends BaseAction {
	
	public function index(){
		$history = file_get_contents(C('HISTORY_URL')."?opt=list&target=".$_GET['zone']."&class=domain");				
		$history_list = json_decode($history,true);
		$list = $history_list['descmap'];
		
		if(!empty($_GET['startTime']) && !empty($_GET['endTime']) && !empty($_GET['val'])){
			foreach($list as $key => $val){
				if(strtotime(substr($val,0,10)) >= strtotime($_GET['startTime']) && strtotime(substr($val,0,10)) <= strtotime($_GET['endTime']) && strstr($val,$_GET['val'])){
					$tem[$key] = $val;
				}
			}
			$list = $tem;
			$this->assign("startTime",$_GET['startTime']);		
			$this->assign("endTime",$_GET['endTime']);	
			$this->assign("val",$_GET['val']);	
		}
		if($history_list['ret'] == 0){			
			$this->assign("count",count($list));	
		}else{
			$this->assign("count",0);	
		}
		
		$kk = 0;
		$i = 0;
		$newlist = array();
		for($o=count($list)-1;$o>=0;$o--){
			$i ++;
			$newlist[$kk][$i] = $list[$o];
			if($i % 10 == 0){
				$kk ++ ;
			}
		}
		$page = 1;
		if(!empty($_GET['p'])){
			$page = $_GET['p'];
		}
		
		if(count($list)<=10){
			$pageCount = 1;
		}else{
			if(count($list) % 10 == 0){
				$pageCount = count($list) / 10;
			}else{
				$pageCount = (count($list) - count($list) % 10) / 10 + 1;
			}
		}
		
		
//		print_r($page);exit;
//		print_r($newlist);exit;
		
		$this->assign("list",$newlist[$page-1]);
		$this->assign("pageCount",$pageCount);
		$this->assign("page",$page);
		$this->assign("sum",count($list));
		$this->assign("count",count($newlist[$page-1]));
		$this->assign("zone",$_GET['zone']);
		$this->display();	
	}

}
