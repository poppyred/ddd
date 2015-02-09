<?php

require_once(dirname(__FILE__).'/mysql.class.php');
ini_set('date.timezone','Asia/Shanghai');

$opt = @$_GET['opt'];
$src = @$_GET['src'];
$dst = @$_GET['dst'];
$oldsrc = @$_GET['oldsrc'];
$olddst = @$_GET['olddst'];

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
case "edit":
	edit($src,$dst,$oldsrc,$olddst);
	break;
default:
	echo "Opt error!\n";
	break;
}


function edit($src,$dst,$oldsrc,$olddst)
{
	$mysql = new MySQL();
	$mysql -> opendb("dns_http_ref", "utf8");
	
	
	$sql = "select * from `http_ref` where `src`='$oldsrc' and `dst`='$olddst'";
	#echo $sql;
	$result = $mysql->query($sql);
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL)
    {
		print_r(json_encode(result_init(1,'数据为空','')));
        exit;
    }
	while( ($row = mysql_fetch_array($result)) ) 
	{
		#print_r($row);
		$id = $row['id']; 
	
	}

	$sql = "update `http_ref` set `src` = '$src',`dst` = '$dst' where `id` = '$id'";
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