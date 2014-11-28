<?php

ini_set('date.timezone','Asia/Shanghai');

$opt = @$_GET['opt'];

#$temp = array('8.8.8.8','9.9.9.9');
#$temp = json_encode($temp,true);
#print_r($temp);
#exit();
/*json*/
$val = @$_GET['val'];

$val = json_decode($val,true);

#$val = array('0'=>'www.eflydns.com');
if (count($val) == 0)
{
	print_r(json_encode(result_init('1','val input wrong!','')));
	exit();
}

$data = '';
$ret = 0;

switch($opt)
{
case 'chart':
	$data = get_chart_data($val);
	if ($data)
	{
		$ret = 0;
	}
	else
	{
		$ret = 1;
		$data = "信息不存在";
	}
	break;
	
case 'add':
	$arr = set_value($val);
	$data = $arr['msg'];
	$ret = $arr['ret'];
	break;
	
case 'del':
	$arr = del_value($val);
	$data = $arr['msg'];
	$ret = $arr['ret'];
	break;
	
case 'status':
	$data = get_httpcode_data($val,$ret);
	/*
	if ($data)
	{
		$ret = 0;
	}
	else
	{
		$ret = 1;
	}
	*/
	break;
default:
	print_r(json_encode(result_init('1','opt input wrong!','')));
	break;
	
}
	

print_r(json_encode(result_init($ret,'',$data)));



function get_chart_data($val)
{
	$total = 0;
	$cnt_500 = 0;
	$cnt_1000 = 0;
	$cnt_el = 0;
	$cnt_0 = 0;

	$val = $val[0];
	
	$ret = array();
	$url = "http://121.201.12.57:8080/dns_monitor_web/query?server=".$val;
	#print_r($url);
	$data = file_get_contents($url);
	#print_r($url)
	#print_r($data);
	$data = json_decode($data,true);
	$cnt = count($data);
	if ($cnt == 0)
	{
		return null;
	}
	else
	{
		ksort($data);
	}
	
	$ret['line'] = $data;
	
	foreach($data as $ms)
	{
		$total += $ms;
		
		if ($ms == 0)
		{
			$cnt_0 ++;
			$cnt --;
		}
		else if ($ms <= 500)
		{
			$cnt_500 ++;
		}
		else if ($ms <= 1000)
		{
			$cnt_1000 ++;
		}
		else
		{
			$cnt_el ++;
		}
	}

	if ($cnt == 0)
	{
		$ret['avg'] = -1;
		$ret['pie'] = array('不可用'=>100);
	}
	else
	{
		$ret['avg'] = round($total/$cnt);
		$ret['pie'] = array('0~500ms'=>round($cnt_500/$cnt,2)*100,'500~1000ms'=>round($cnt_1000/$cnt,2)*100,'大于1000ms'=>round($cnt_el/$cnt,2)*100,'不可用'=>round($cnt_0/$cnt,2)*100);
	}
	
	
	#$ret['httpcode'] = get_httpcode_data($val);
	#print_r($ret);
	return $ret;
}

function get_httpcode_data($val,&$ret)
{
	
	foreach($val as $tar)
	{
		
		$url = "http://121.201.12.57:8080/dns_monitor_web/httpcode?server=".$tar;
		$httpcode = file_get_contents($url);
		
		#print_r($url);
		
		$httpcode = json_decode($httpcode,true);
		
		#print_r($httpcode);
		
		$status = $httpcode['ret'];
		$msg = $httpcode['msg'];
	
		$ret = $status;
		
		if ($msg == '200')
		{
			$r_msg = '200(正常访问)';
		}
		else if ($msg == '404')
		{
			$r_msg = '404(无法访问)';
			break;
		}
		else if ($msg == '403')
		{
			$r_msg = '403(访问受限)';
			break;
		}
		else
		{
			$r_msg = "异常";
			break;
		}
	}
	
	#print_r($r_msg);
	return $r_msg;
}


function set_value($val)
{
	$val = $val[0];
	
	#print_r($val);
	$url = "http://121.201.12.57:8080/dns_monitor_web/add?server=".$val;
	#print_r($url);
	$ret = file_get_contents($url);
	#print_r($ret);
	$ret = json_decode($ret,true);

	return $ret;
	
}

function del_value($val)
{
	$val = $val[0];
	$url = "http://121.201.12.57:8080/dns_monitor_web/delete?server=".$val;
	$ret = file_get_contents($url);
	
	$ret = json_decode($ret,true);
	return $ret;
}



function result_init($ret,$error,$data)
{
	$json_result = array();
	$json_result['ret'] = $ret;
	$json_result['error'] = $error;
	$json_result['descmap'] = $data;
	
	return $json_result;
	
}

?>