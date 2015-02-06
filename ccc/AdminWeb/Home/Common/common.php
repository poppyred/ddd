<?php
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
	
	function ip_to_num($ip){
		$ip_arr=explode('.',$ip);//分隔ip段
		foreach ($ip_arr as $value)
		{
			$iphex=dechex($value);//将每段ip转换成16进制
			if(strlen($iphex)<2)//255的16进制表示是ff，所以每段ip的16进制长度不会超过2
			{
				$iphex='0'.$iphex;//如果转换后的16进制数长度小于2，在其前面加一个0
			//没有长度为2，且第一位是0的16进制表示，这是为了在将数字转换成ip时，好处理
			}
			$ipstr.=$iphex;//将四段IP的16进制数连接起来，得到一个16进制字符串，长度为8
		}
		return hexdec($ipstr);//将16进制字符串转换成10进制，得到ip的数字表示
	}
	 
	 
	//将数字转换为IP，进行上面函数的逆向过程
	function num_to_ip($n){
		$iphex=dechex($n);//将10进制数字转换成16进制
		$len=strlen($iphex);//得到16进制字符串的长度
		if(strlen($iphex)<8)
		{
			$iphex='0'.$iphex;//如果长度小于8，在最前面加0
			$len=strlen($iphex); //重新得到16进制字符串的长度
		}
		//这是因为ipton函数得到的16进制字符串，如果第一位为0，在转换成数字后，是不会显示的
		//所以，如果长度小于8，肯定要把第一位的0加上去
		//为什么一定是第一位的0呢，因为在ipton函数中，后面各段加的'0'都在中间，转换成数字后，不会消失
		for($i=0,$j=0;$j<$len;$i=$i+1,$j=$j+2)
		{//循环截取16进制字符串，每次截取2个长度
			$ippart=substr($iphex,$j,2);//得到每段IP所对应的16进制数
			$fipart=substr($ippart,0,1);//截取16进制数的第一位
			if($fipart=='0')
			{//如果第一位为0，说明原数只有1位
				$ippart=substr($ippart,1,1);//将0截取掉
			}
			$ip[]=hexdec($ippart);//将每段16进制数转换成对应的10进制数，即IP各段的值
		}
		$ip = array_reverse($ip);
		 
		return implode('.', $ip);//连接各段，返回原IP值
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