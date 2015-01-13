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
		
		$this->assign("list",$list);		
		$this->assign("zone",$_GET['zone']);
		$this->display();	
	}

}
