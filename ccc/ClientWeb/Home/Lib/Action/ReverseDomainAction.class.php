<?php
// 本类由系统自动生成，仅供测试用途
class ReverseDomainAction extends BaseAction {
	public function index(){
		if(!empty($_GET['id'])){
			$reverse = M('reverse_domain');
			$rlist = $reverse->where('client_id='.$_GET['id'])->select();
			
			$this->assign('rlist',$rlist);
			$this->display();	
		}
	}	
	public function deleteReverse(){
		if(!empty($_POST['id'])){			
			$reverse = M('reverse_domain');
			$is_ok = $reverse->where('id='.$_POST['id'])->delete();
			if($is_ok === false){
				$this->ajaxReturn('删除域名反解析失败，请联系管理员','error',0);
			}
			$this->ajaxReturn('删除域名反解析成功','success',1);
		}
	}	
}

