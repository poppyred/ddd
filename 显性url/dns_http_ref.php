<?php

require_once(dirname(__FILE__).'/mysql.class.php');
ini_set('date.timezone','Asia/Shanghai');


$opt = @$_GET['opt'];
$src = @$_GET['src'];
$dst = @$_GET['dst'];

if (!isset($opt) || !isset($src) || !isset($dst))
{
	echo "Argv error!\n";
	exit;
}

switch($opt)
{
case 'add':
	add($src,$dst);
	break;
case "del":
	del($src,$dst);
	break;
default:
	echo "Opt error!\n";
	break;
}


function add($src,$dst)
{
	$mysql = new MySQL();
	$mysql -> opendb("dns_http_ref", "utf8");
	
	$sql = "insert into `http_ref` values('','$src','$dst','true')";

	$result = $mysql->query($sql);
	if ($result)
	{
		$ret = 0;
	}
	else
	{
		$ret = 1;
	}
	
	print_r(json_encode(result_init($ret,'','')));
	
}


function del($src,$dst)
{
	$mysql = new MySQL();
	$mysql -> opendb("dns_http_ref", "utf8");
	
	#$sql = "update `http_ref` set `status` = 'false' where `dst`='$dst' and `src`='$src'";
	$sql = "delete from `http_ref` where `dst`='$dst' and `src`='$src'";

	$result = $mysql->query($sql);
	if ($result)
	{
		$ret = 0;
	}
	else
	{
		$ret = 1;
	}
	
	print_r(json_encode(result_init($ret,'','')));
	
}


//END

function result_init($ret,$error,$data)
{
	$json_result = array();
	$json_result['ret'] = $ret;
	$json_result['error'] = $error;
	$json_result['descmap'] = $data;
	
	return $json_result;
	
}


?>