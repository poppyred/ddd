<?php
	function writeLog($msg){
		$logFile = date('Y-m-d').'.txt';
		$msg = date('Y-m-d H:i:s').' >>> '.$msg."\r\n";
		file_put_contents($logFile,$msg,FILE_APPEND );
	}
	/**
	+----------------------------------------------------------
	* 写操作日志
	+----------------------------------------------------------
	*/
	function WriteOptLog($type, $obj){
		$List = M("optlog");
		$data['opt_zone'] = $_SESSION['zone'];
		$data['opt_time'] = date("Y-m-d H:i:s", time());
		$data['opt_user'] = $_SESSION['user'];
		$data['opt_type'] = $type;
		$data['opt_obj'] = $obj;
				
		$re = $List->data($data)->add();
	}
	
	function http_post($url, $data)
        {
		//$data=array();
		$data = array('data' => json_encode($data));
		$data_url = http_build_query ($data);
		$data_len = strlen ($data_url);

		return array('content'=>file_get_contents ($url, false, stream_context_create (array ('http'=>array ('method'=>'POST'
                 , 'header'=>"Connection: close\r\nContent-Length: $data_len\r\n"
                 , 'content'=>$data_url
                 ))))
                , 'headers'=>$http_response_header
                );
	}
	
	/**
	+----------------------------------------------------------
	* object和array的相互转换
	+----------------------------------------------------------
	* @access public
	+----------------------------------------------------------
	* @param $array 参数
	* $data=object_array($list);
	+----------------------------------------------------------
	* @return array
	+----------------------------------------------------------
	*/
	function object_array($array){
		if(is_object($array)){
			$array = (array)$array;
		}
		if(is_array($array)){
			foreach($array as $key=>$value){
				  $array[$key] = object_array($value);
			}
		}
		return $array;
	} 

	/**
	+----------------------------------------------------------
	* 合并两个数组函数
	+----------------------------------------------------------
	* @access public
	+----------------------------------------------------------
	* @param array $list1 数组名
	* @param array $list2 数组名
	* $username=MergeArray($list1,$list2);
	+----------------------------------------------------------
	* @return array
	+----------------------------------------------------------
	*/
	function MergeArray($list1,$list2){
		if(!empty($list1) && !empty($list2)) 
		{
			return array_merge($list1,$list2);
		}
		else return (empty($list1)?(empty($list2)?null:$list2):$list1);
	}
	
	/**
	+----------------------------------------------------------
	* 单条件查询数据库函数
	+----------------------------------------------------------
	* @access public
	+----------------------------------------------------------
	* @param string $table 查询表名
	* @param string $fields 返回的字段名
	* @param string $id  条件字段的值
	* @param string $str 条件字段的名称
	* $username=Getx2x('User','username','12'); 或者
	* $username=Getx2x('User','username','12','id');
	+----------------------------------------------------------
	* @return string or array
	+----------------------------------------------------------
	*/
	function Getx2x($table, $fields, $id, $str){
		$tb=M($table);
		if(empty($str)){
			$expression='getByid';
		}else{
			$expression='getBy'.$str;
		}
		$rlt = $tb->field($fields)->$expression($id);
		$arr = explode(',',$fields);
		if(count($arr) <= 1){
			return $rlt[$fields];
		}else{
			return $rlt;
		}
	}
	
	/**
	+----------------------------------------------------------
	* 对查询结果集进行排序
	+----------------------------------------------------------
	* @access public
	+----------------------------------------------------------
	* @param array $list 查询结果
	* @param string $field 排序的字段名
	* @param string $sortby 排序类型
	* asc正向排序 desc逆向排序 nat自然排序
	+----------------------------------------------------------
	* @return array
	+----------------------------------------------------------
	*/
	function list_sort_by($list, $field, $sortby='asc') {
		if(is_array($list)){
			$refer = $resultSet = array();
			foreach ($list as $i => $data)
				$refer[$i] = &$data[$field];
			switch ($sortby) {
				case 'asc': // 正向排序
					asort($refer);
					break;
				case 'desc':// 逆向排序
					arsort($refer);
					break;
				case 'nat': // 自然排序
					natcasesort($refer);
					break;
			}
			foreach ( $refer as $key=> $val)
				$resultSet[] = &$list[$key];
			return $resultSet;
		}
		return false;
	}
	
?>