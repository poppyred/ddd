<?php
// 本类由系统自动生成，仅供测试用途
class ReverseDomainAction extends BaseAction {
	public function index(){
		$this->assign('rlist',$this->returnList());		
		$view = M('view');
		$viewList = $view->select();
		$this->assign('viewList',$viewList);
		$this->display();	
	}	
	public function deleteReverse(){
		if(!empty($_POST['id'])){
			
			$reverse = M('reverse_domain');
			$data = $reverse->where('id='.$_POST['id'])->find();
			$val = array("rid"=>(int)$_POST['id'], 'PTR'=>$data['domain']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
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
			$this->ajaxReturn($this->returnList(),'success',1);				
		}
	}	
	public function addReverse(){
		if(!empty($_POST['ip'])){			
			$reverse = M('reverse_domain');
			$entity = $reverse->where("ip='".$_POST['ip']."' and domain='".$_POST['domain']."' and view_id=".$_POST['view']." and client_id=".$_SESSION['id'])->find();
			if(!empty($entity)){
				$this->ajaxReturn('请不要重复添加相同的域名反解析','error',0);
			}
			$data['ip'] = $_POST['ip'];
			$data['domain'] = $_POST['domain'];
			$data['client_id'] = $_SESSION['id'];
			$data['view_id'] = $_POST['view'];
			$is_ok = $reverse->add($data);
			if($is_ok === false){
				$this->ajaxReturn('添加域名反解析失败，请联系管理员','error',0);
			}
			
			$val = array("name"=>strtolower($_POST['ip']), "rid"=>(int)$is_ok, 'PTR'=>rawurlencode($_POST['domain']), "ttl"=>(int)(10)*60, "viewid"=>$_POST['view']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"record", "opt"=>"add", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] != 0){
				$this->ajaxReturn(0,'添加域名反解析失败，请联系管理员',0);
			}
			
			$this->ajaxReturn($this->returnList(),'success',1);
		}	
	}
	public function updateReverse(){
		if(!empty($_POST['id']) && !empty($_POST['ip'])){
			$reverse = M('reverse_domain');
			$is_ok = $reverse->where('id='.$_POST['id'])->setField(array('ip'=>$_POST['ip'],'domain'=>$_POST['domain'],'view_id'=>$_POST['view']));
			if($is_ok === false){
				$this->ajaxReturn('修改域名反解析失败，请联系管理员','error',0);
			}
			
			$val = array("name"=>strtolower($_POST['ip']), "rid"=>(int)$_POST['id'], 'PTR'=>rawurlencode($_POST['domain']), "ttl"=>(int)(10)*60, "viewid"=>$_POST['view']);
			$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
			$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
			$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
			$rslt = json_decode($ret["content"],true);
			if($rslt["ret"] != 0){
				$this->ajaxReturn(0,'修改域名反解析失败，请联系管理员',0);
			}
			$this->ajaxReturn($this->returnList(),'success',1);
		}
	}
	public function startStopReverse(){
		if(!empty($_POST['id']) && isset($_POST['status'])){
			$reverse = M('reverse_domain');
			$is_ok = $reverse->where(array('id'=>array('in',$_POST['id'])))->setField('status',$_POST['status']);			
			$rlist = $reverse->where(array('id'=>array('in',$_POST['id'])))->select();
			if($_POST['status']==0){
				if($is_ok === false){
					$this->ajaxReturn('启用域名反解析失败，请联系管理员','error',0);
				}				
				foreach($rlist as $r){
					$val = array("name"=>strtolower($r['ip']), "rid"=>(int)$r['id'], 'PTR'=>rawurlencode($r['domain']), "ttl"=>(int)(10)*60, "viewid"=>$r['view_id'],"enable"=>1);					
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
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
					$user = array("cid"=>$_SESSION['id'], "level"=>0, "info"=>"");
					$param = array("type"=>"record", "opt"=>"set", "data"=>$val,"user"=>$user);
					$ret = http_post(C('INTERFACE_URL')."/dnspro/dnsbroker/", $param);
					$rslt = json_decode($ret["content"],true);
					if($rslt["ret"] != 0){
						$this->ajaxReturn($rslt["error"],'error',0);
					}
				}			
			}
			$this->ajaxReturn($this->returnList(),'success',1);
		}
	}	
	
	public function returnList(){		
		$reverse = M('reverse_domain');
		$view = M('view');
		$rlist = $reverse->where('client_id='.$_SESSION['id'])->select();
		foreach($rlist as $key => $val){
			$tem = $view->where('id='.$val['view_id'])->find();
			$rlist[$key]['view_name'] = $tem['name'];
		}
		
		return $rlist;
	}
}

