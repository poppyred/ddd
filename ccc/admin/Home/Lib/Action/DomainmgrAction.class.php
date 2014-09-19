<?php
// 本类由系统自动生成，仅供测试用途
class DomainmgrAction extends Action {

	public function index(){
		$list = M('zone',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->where('level < 8')->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$zlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where('level < 8')->select();
		//print_r($list->getlastsql());exit;
		$show       = $Page->show();// 分页显示输出
		$this->assign('zlist', $zlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function vip(){
		$list = M('zone',Null,'DB_NEWS'); 
		import('ORG.Util.Page');// 导入分页类
		$count      = $list->where('level >= 8')->count();// 查询满足要求的总记录数 $map表示查询条件
		$Page       = new Page($count, 20);// 实例化分页类 传入总记录数
		$nowPage = isset($_GET['p'])?$_GET['p']:1;
		// 进行分页数据查询 注意page方法的参数的前面部分是当前的页数使用 $_GET[p]获取
		$zlist = $list->order('id desc')->page($nowPage.','.$Page->listRows)->where('level >= 8')->select();
		//print_r($list->getlastsql());exit;
		$show       = $Page->show();// 分页显示输出
		$this->assign('zlist', $zlist);// 赋值数据集
		$this->assign('page',$show);// 赋值分页输出
		$this->display();
	}
	
	public function readZone(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$tb = M('zone',Null,'DB_NEWS'); 
			$ret = $tb->where('id='.$_POST['id'])->find();
			//print_r($tb->getlastsql());exit;
			$this->ajaxReturn($ret,'',1);
		}
	}

	public function readInfo(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$list = M('client');
			$data = $list->where('id='.$_POST['id'])->find();
			$this->ajaxReturn($data,'',1);
		}
	}
	
	public function delDomain(){
		if($_SERVER['REQUEST_METHOD' ] === 'POST'){
			$client_domain = M('client_domain');
			$domain = M('domain');
			$domain->where('id='.$_POST['id'])->delete();
			$client_domain->where('domain_id='.$_POST['id'])->delete();
			$this->ajaxReturn(1,'域名已经删除！',1);
		}
	}
	
	
}
