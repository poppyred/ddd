<?php
// 本类由系统自动生成，仅供测试用途
class MonitorAction extends BaseAction {

	public function index(){
		$this->display();
	}
	
	public function detail(){
		if(!empty($_GET['d'])){
			//print_r($_GET['d']);exit;
			$List = M("zone");
			$condition['domain'] = $_GET['d'];
			$domain = $List->where($condition)->find();
			
			if(!empty($domain)){
				$cnd["zone_id"] = $domain["id"];
				$record = M("domain")->where($cnd)->select();
				$this->assign('recordlist', $record);
			}
			//echo $List->getlastsql();exit;
			//print_r($list);exit;
			$this->assign('domain', $_GET['d']);
			$this->display();
		}
	}
}
