<?php
// 本类由系统自动生成，仅供测试用途
class ClientmgrAction extends Action {

	public function index(){
		$list = M('client',Null,'DB_NEWS'); 
		$map["type"] = 0;
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$clist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		//print_r($list->getlastsql());exit;
		$show       = $Page->show();// 分页显示输出
		$this->assign('clist',$clist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function vip(){
		$list = M('client',Null,'DB_NEWS'); 
		$map["type"] = 1;
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->where($map)->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$clist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where($map)->select();
		$show       = $Page->show();// 分页显示输出
		$this->assign('clist',$clist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	

	public function readInfo(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('client',Null,'DB_NEWS'); 
			$data = $list->where('id='.$_POST['id'])->find();
			$this->ajaxReturn($data,'',1);
		}
	}
	
	public function delClient(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$client_domain = M('client_domain',Null,'DB_NEWS');
			$domain_list = $client_domain->where('client_id='.$_POST['id'])->select();
			if(!empty($domain_list)){
				$domain = M('domain',Null,'DB_NEWS');
				foreach($domain_list as $val){
					$domain->where('id='.$val['domain_id'])->delete();
				}
			}
			$client_domain->where('client_id='.$_POST['id'])->delete();
			
			$zone = M('zone',Null,'DB_NEWS');
			$zone_list = $zone->where('client_id='.$_POST['id'])->select();
			if(!empty($zone_list)){
				$zone_level = M('zone_level',Null,'DB_NEWS');
				$zone_name = M('zone_name',Null,'DB_NEWS');
				$route = M('route',Null,'DB_NEWS');
				foreach($zone_list as $val){
					//删除zone_level
					$zone_level->where('zone_id='.$val['id'])->delete();
					//删除zone_name
					$zone_name->where('zone_id='.$val['id'])->delete();
					//删除线路的掩码
					$route_list = $route->where('zone_id='.$val['id'])->select();
					if(!empty($route_list)){
						$route_mask = M('route_mask',Null,'DB_NEWS');
						foreach($route_list as $v){
							$route_mask->where('route_id='.$v['id'])->delete();
						}
					}
					//删除自定义路线
					$route->where('zone_id='.$val['id'])->delete();
				}
			}
			$zone->where('client_id='.$_POST['id'])->delete();
			
			$group = M('client_group',Null,'DB_NEWS');
			$group->where('client_id='.$_POST['id'])->delete();
			
			$list = M('client',Null,'DB_NEWS');
			$list->where('id='.$_POST['id'])->delete();
			$this->ajaxReturn(1,'用户已经删除！',1);
		}
	}
	
	
}
